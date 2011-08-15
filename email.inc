<?php

/**
 * @file
 * This is page that processes the email functions
 * TODO: validate functions for each form
 */

/**
 * Page Callback and Functions for Email
 */

/**
 * Get groups returns a 2 dimensionsal array
 * with the parent title as the first dimension
 * key and the gid as the second dimension key
 * and the value is set to the group title.
 *
 * @return
 * 	$parent
 */
function _getGroups() {
	$query = db_select('email_groups', 'g');
	$query->join('email_parents', 'p', 'g.pid = p.pid');

	$query
		->fields('p', array('pid', 'ptitle'))
		->fields('g', array('gid', 'gtitle', 'pid'))
		->groupBy('gid')
		->orderBy('gtitle', 'ASC');

	$result = $query->execute();

	$parent = array();

 //Add Default Groups

   $parent['Default Lists']['A'] = t('Active Members');
			$parent['Default Lists']['B'] = t('Alumni Members');
			$parent['Default Lists']['C'] = t('General Board');
			$parent['Default Lists']['D'] = t('Webmaster');

	foreach ($result as $group) {
		$parent[$group->ptitle][$group->gid] = $group->gtitle;
	}

	return $parent;
}


/**
 * Returns a form to email company members.
 *
 * This function creates a form array with
 * two entry options: email_recipents, and
 * email_content. Email_recipents is a select
 * list of groups, and email_content is a
 * text_format field for the content of the
 * email.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return
 * 	$form
 */
function tasks_email($form, &$form_state) {
  $form = array();

		$form = array('#attributes' => array('enctype' => "multipart/form-data"),);

		$form['headers'] = array(
			'#type' => 'fieldset',
			'#title' => t('Headers'),
		);

  $form['headers']['email_from'] = array(
				'#type' => 'item',
				'#title' => t('From'),
				'#required' => TRUE,
				'#markup' => t('The <em>From:</em> address will be automatically generated based up on the Recipient group.'),
		);

  $form['headers']['email_recipents'] = array(
				'#type' => 'select',
				'#title' => t('Recipients'),
	   '#options' => _getGroups(),
				'#required' => TRUE,
  );

  $form['headers']['email_subject'] = array(
				'#type' => 'textfield',
				'#title' => t('Email Subject'),
				'#required' => TRUE,
		);

		$form['content'] = array(
			'#type' => 'fieldset',
			'#title' => t('Content'),
		);

  $form['content']['email_body'] = array(
   '#type' => 'textarea',
			'#title' => t('Email Body'),
			'#rows' => 20,
			//'#format' => 'full_html',
			'#required' => TRUE,
  );

		/*$form['content']['email_attach'] = array(
			'#title' => t('Email Attachments'),
   '#type' => 'file',
   '#size' => 22,
   '#theme_wrappers' => array(),
		);*/

		$form['reqexpl'] = array(
    '#markup' => t('Elements with an astrick (') . '<span class="form-required" title="This field is required.">' . t('*') . '</span>' . t(') are required.'),
    '#prefix' => '<p><br />',
    '#suffix' => '</p>',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send Email'),
				'#prefix' => '',
  );

  return $form;
}

function tasks_email_validate($form, &$form_state) {

}

function tasks_email_submit($form, &$form_state) {
	/**
	* Process for when email system is off
	* TODO: Rethink this process (what really needs to happen?)
	*/
	if(variable_get('tasks_email_status') == 'Off') {
			watchdog('Tasks', t('Message attempt.'), array(), WATCHDOG_DEBUG, $_SERVER['PHP_SELF']);
			drupal_set_message(t('The Email System is currently turned off. Contact the webmaster to have it activated.'), 'error');
	}

	/**
	* Processes for when email system is On
	*/
	if(variable_get('tasks_email_status') == 'On') {
		/**
		* First Step: Get the Email Addresses based on the form input
		*/

		// Active Members
		if( $form_state['values']['email_recipents'] == 'A' ) {
			$query =	db_select('users', 'u');
			$query->join('users_roles', 'r', 'u.uid = r.uid');
			$query
				->condition('r.rid', '9')
				->fields('u', array('mail'));

			$results = $query->execute();

			$email = array();
			foreach ($results as $r){
				$email[] .= $r->mail;
			}

		}

		// Alumni Members
		if( $form_state['values']['email_recipents'] == 'B' ) {
			$query =	db_select('users', 'u');
			$query->join('users_roles', 'r', 'u.uid = r.uid');
			$query
				->condition('r.rid', '10')
				->fields('u', array('mail'));

			$results = $query->execute();

			foreach ($results as $r){
				$email[] .= $r->mail;
			}
		}

		// Gen Board
		if( $form_state['values']['email_recipents'] == 'C' ) {
			// Get Gen Board Members for Current Season
			$query = db_select('board_involve', 'b');
			$query
				->condition('season_id', variable_get('season_current', NULL))
				->fields('b', array('uid'));

			$results = $query->execute();

			foreach ($results as $r) {
				$users[] = $r->uid;
			}

			// Get email addresses
			$results = db_query("SELECT `mail` FROM {users} WHERE nid IN (:nids)", array(':nids' => $users));

			foreach ($results as $r){
				$email[] .= $r->mail;
			}
		}

		// Webmaster
		if( $form_state['values']['email_recipents'] == 'D' ) {
			$query =	db_select('users', 'u');
			$query->join('users_roles', 'r', 'u.uid = r.uid');
			$query
				->condition('r.rid', '3')
				->fields('u', array('mail'));

			$results = $query->execute();

			$email = array();
			foreach ($results as $r){
				$email[] .= $r->mail;
			}
		}

		// Else: Get Email_members from table
		else {
			$query = db_select('email_members', 'm');
			$query
				->condition('gid', $form_state['values']['email_recipents'])
				->fields('m', array('email'));

			$results = $query->execute();

			$email = array();
			foreach ($results as $r){
				$email[] .= $r->email;
			}
		}

		/**
		* Second Step: Build and Send the Email
		*/

		try {
			global $user;

			$module = 'tasks';
			$key = 'Email';
			if (isset($from_dept)) {
				if ($from_dept == '1') {
					$key = 'Info Email';
				}
				if ($from_dept == '2') {
					$key = 'Outreach Email';
				}
				if ($from_dept == '3') {
					$key = 'Special Events Email';
				}
				if ($from_dept == '4') {
					$key = 'Box Office Email';
				}
			}
			$language = language_default();
			$params = array();
			$from = 'The Livingston Theatre Company <info@rultc.org>';
			$send = FALSE;
			$message = drupal_mail($module, $key, $email, $language, $params, $from, $send);

			// Set some default header values
			$message['headers'] = array(
				'MIME-Version'                 =>  '1.0',
				'Content-Type'                 =>  'text/plain; charset=UTF-8; format=flowed',
				'Content-Transfer-Encoding'    =>  '8Bit',
				'X-Mailer'                     =>  'Drupal - ' . VERSION,
			);

			// Work with the from header so the email comes from the appropriate department
			$query = db_select('email_groups', 'g');
			$query->join('email_parents', 'p', 'g.pid = p.pid');
			$query
				->fields('p', array('pid'))
				->condition('gid', $form_state['values']['email_recipents']);

			$results = $query->execute();

			foreach ($results as $row) {
				$from_dept = $row->pid;
			}

			if (isset($from_dept)) {
				if ($from_dept == '1') {
					$message['headers']['From'] = 'The Livingston Theatre Company <info@rultc.org>';
				}
				if ($from_dept == '2') {
					$message['headers']['From'] = 'The Livingston Theatre Company - Outreach Department <outreach@rultc.org>';
				}
				if ($from_dept == '3') {
					$message['headers']['From'] = 'The Livingston Theatre Company - Special Events Department <events@rultc.org>';
				}
				if ($from_dept == '4') {
					$message['headers']['From'] = 'The Livingston Theatre Company - Box Office <boxoffice@rultc.org>';
				}
			}

			else {
				$message['headers']['From'] = 'The Livingston Theatre Company <info@rultc.org>';
			}

			// Set addresses, subject, ect
			$temp_to = $email;
			$message['to'] = implode(",", $temp_to);

			$message['subject'] = $form_state['values']['email_subject'];

			// Work with body to make lines end with '\r\n'
			$temp_body = $form_state['values']['email_body'];
			$body_array = explode('\r\n', $temp_body);
			$message['body'] = $body_array;

			// Retrieve the responsible implementation for this message.
			$system = drupal_mail_system($module, $key);

			// Format the message body.
			$message = $system->format($message);

			// Send e-mail.
			$message['result'] = $system->mail($message);

			// Log errors
			if (!$message['result']) {
				watchdog('mail', 'Error sending e-mail to %to', array('%to' => count($message['to'])), WATCHDOG_ERROR);
				drupal_set_message(t('Unable to send e-mail. Contact the site administrator if the problem persists.'), 'error');
			}

				drupal_set_message(t('Email with subject <em> @subject </em> sent to @number people.', array('@subject' => $form_state['values']['email_subject'], '@number' => count($email))));
				watchdog('Tasks', 'Message @subject sent', array('@subject' => $form_state['values']['email_subject']), WATCHDOG_INFO);
		}

		catch (Exception $e) {
			watchdog('php', $e);
			drupal_set_message(t('Error sending email. Contact webmaster.'), 'error');
		}
	}
}

/**
 * Page Callback and Functions for Email>Groups
 */

/**
 * Returns a table of email groups
 *
 * This function builds a table of email groups
 * based on the content of {email_groups},
 * {email_parents}, and {email_members}.
 *
 * @return str $output
 *
 */
function tasks_groups_hello() {
	$query = db_select('email_groups', 'g');
	$query->join('email_parents', 'p', 'g.pid = p.pid');

 $query
  ->fields('g', array('gtitle', 'gid'))
  ->fields('p', array('pid', 'ptitle'))
  //->groupBy('g.gid')
  ->orderBy('g.gid', 'DESC');

	$result = $query->execute();

	$rows = array();

	foreach ($result as $entry) {
		// Sanitize the data before handing it off to the theme layer.
		$rows[] = array(check_plain($entry->gtitle), $entry->ptitle, l('View Members', 'email/groups/list/' . $entry->gid));
	}

	// Make a table for them.
	$header = array(t('Group Title'), t('Parent Group'), t('Member Count'));
	$output = theme('table', array('header' => $header, 'rows' => $rows));

	return $output;
}

/**
 * Page Callback and Functions for Email>Groups>Add
 */

function tasks_groups_add($form, &$form_state) {
 	global $user;
 	$form = array();

		// Use fieldset in values when accessing for processing
	 $form['#tree'] = TRUE;

  if (empty($form_state['num_addresses'])) {
    $form_state['num_addresses'] = 1;
  }

	 $form['title'] = array(
		  '#title' => t('Group Title'),
		  '#description' => t('Be descriptive! Example- "Fall Involvememnt Fair 2010"'),
		  '#type' => 'textfield',
	 );

	 $form['parent'] = array(
		  '#title' => t('Group Parent'),
		  '#description' => t('Parent Category to organize dropdown on email page'),
		  '#type' => 'select',
		  '#options' => array('1' => 'Default Groups', '2' => 'Outreach', '3' => 'Speical Events', '4' => 'Box Office'),
		  '#default_value' => 'Default Groups',
	 );

	 $form['email'] = array(
				'#type' => 'fieldset',
				'#title' => t('Member Information'),
	 );

	 for ($i = 1; $i <= $form_state['num_addresses']; $i++) {
				$form['email'][$i]['name'] = array(
								'#prefix' => '<div style="width: 50%; float: left">',
								'#suffix' => '</div>',
								'#type' => 'textfield',
								'#size' => 50,
								'#description' => t('Enter a name to associate with the email'),
								'#title' => t('Name @num', array('@num' => $i))
				);
				$form['email'][$i]['address'] = array(
				  '#prefix' => '<div style="width: 50%; float: left">',
				  '#suffix' => '</div>',
					 '#title' => t('Address @num', array('@num' => $i)),
					 '#description' => t('Enter an email to add to the selected list'),
				  '#size' => 50,
					 '#type' => 'textfield',
					 '#element_validate' => 'valid_email_address',
				);
  }

  // Adds "Add More" button
  $form['email']['add_more'] = array(
    '#type' => 'submit',
    '#value' => t('Add More'),
    '#submit' => array('tasks_add_more'),
  );

  $form['uid'] = array(
		  '#type' => 'hidden',
		  '#value' => $user->uid,
	  );

	 $form['submit'] = array(
		  '#type' => 'submit',
		  '#value' => t('Create Group'),
	  );

 	return $form;
}

/**
 * Submit handler for "Add More" button on tasks_group_add().
 */
function tasks_add_more($form, &$form_state) {
  // Everything in $form_state is persistent, so we'll just use
  // $form_state['add_name']
  $form_state['num_addresses']++;

  // Setting $form_state['rebuild'] = TRUE causes the form to be rebuilt again.
  $form_state['rebuild'] = TRUE;
}

/**
 * Validate handdler for group_add form
 */
function tasks_groups_add_validate($form, &$form_state) {
	$query = db_select('email_groups', 'e');
	$query
		->fields('e', array('gtitle'))
		->condition('gtitle', $form_state['values']['title']);

	$results = $query->execute();

	$numrows = $results->rowCount();

	if($numrows > 1) {
		form_set_error('title', t('This title is already in use. Please rename your group.'));
	}
}

function tasks_groups_add_submit($form, &$form_state) {
  $txn = db_transaction();

  try {
    // Add the group title to the database
				$query = db_insert('email_groups')
				  ->fields(array(
								'gid' => NULL,
								'gtitle' => $form_state['values']['title'],
								'pid' => $form_state['values']['parent'],
								'owner_id' => $form_state['values']['uid'],
				))
				->execute();

				$query = 'SELECT gid, gtitle FROM {email_groups} WHERE gtitle = \'' . $form_state['values']['title'] . '\' LIMIT 1';
				$results = db_query($query);
				foreach($results as $r) {
								$title = $r->gid;
				}

				for ($i = 1; $i <= $form_state['num_addresses']; $i++) {
				  $email = $form_state['values']['email'][$i]['address'];
				  $name = $form_state['values']['email'][$i]['name'];
      $query = 'INSERT INTO {email_members} (eid, gid, name, email) VALUES (NULL, \' ' . $title . ' \', \' ' . $name . ' \' , \' ' . $email . ' \') ';
						$results = db_query($query);
    }

				watchdog('tasks', t('Email group @group created', array('@group' => $form_state['values']['title'])));
				drupal_set_message(t('The form has been submitted. Group <em>@group</em> created.', array('@group' => $form_state['values']['title'])));
  }

  catch (Exception $e) {
    $txn->rollback();
    watchdog_exception('php', $e);
    drupal_set_message(t('There was a problem creating this email list. Please contact the webmaster.'), 'error');
  }
}

/**
 * Page Callback and Functions for Email>Groups>Edit
 * TODO: Change
 */
function tasks_groups_edit() {
  return(array(
				'intro' => array(
								'#type' => 'markup',
								'#markup' => t('Edit Group Page. This page is still under construction. If you need a member of a group removed or edited, contact Jerry and he will do it for you'),
				),
				//'form' => drupal_get_form('tasks_groups_edit_form'),
		));
}

function _getGroupsEdit() {
	$query = db_select('email_groups', 'g');
	$query->join('email_parents', 'p', 'g.pid = p.pid');

	$query
		->fields('p', array('pid', 'ptitle'))
		->fields('g', array('gid', 'gtitle', 'pid'))
		->groupBy('gid')
		->orderBy('gtitle', 'ASC');

	$result = $query->execute();

	$parent = array();

	foreach ($result as $group) {
		$parent[$group->ptitle][$group->gid] = $group->gtitle;
	}

	return $parent;
}

function tasks_update_ajax($form, &$form_state) {
  (int) $gid = $form_state['values']['group_id'];

  $query = db_select('email_members', 'e');
		$query
		  ->condition('gid', $gid, '=')
				->fields('e', array('email'));

  $num_rows = $query->countQuery();

  $form_state['num_addresses'] = $num_rows;
		$form_state['gid'] = $form_state['values']['group_id'];

		$results = $query->execute();

		foreach ($results as $row) {
				for ($i = 1; $i <= $num_rows; $i++) {
				  $form_state['email'][$i] = $row->email;
				}
  }
}

function tasks_groups_edit_form() {
	global $user;
	$form = array(
				'#prefix' => '<div id="ajaxwrapper">',
    '#suffix' => '</div>'
  );

	$form['group_id'] = array(
		'#type' => 'select',
		'#title' => t('Group to Edit'),
		'#options' => _getGroupsEdit(),
		'#default_value' => isset($form_state['gid']) ? $form_state['gid'] : ' ',
		'#ajax' => array(
				'callback' => 'tasks_update_ajax',
				'wrapper' => 'ajaxwrapper',
		),
		'#required' => TRUE,
	);

  $form['email'] = array(
    '#type' => 'fieldset',
				'#title' => t('Email Addresses'),

	 );

	 for ($i = 1; $i <= $form_state['num_addresses']; $i++) {
				$form['email'][$i]['name'] = array(
								'#prefix' => '<div style="width: 50%; float: left">',
								'#suffix' => '</div>',
								'#type' => 'textfield',
								'#description' => t('Enter a name to associate with the email'),
								'#title' => t('Name @num', array('@num' => $i))
				);
				$form['email'][$i]['address'] = array(
				  '#prefix' => '<div style="width: 50%; float: left">',
				  '#suffix' => '</div>',
					 '#title' => t('Address @num', array('@num' => $i)),
					 '#description' => t('Enter an email to add to the selected list'),
					 '#type' => 'textfield',
						'#default_value' => isset($form_state['email'][$i]) ? $form_state['email'][$i] : ' ',
					 '#element_validate' => 'valid_email_address',
				);
  }

			// Adds "Add More" button
  $form['email']['add_more'] = array(
    '#type' => 'submit',
    '#value' => t('Add More'),
    '#submit' => array('tasks_add_more'),
  );


	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Update Group'),
	);

	return $form;
}

function tasks_list_members($gid = '') {
				if (is_numeric($gid) && isset($gid)) {
				  $query = db_select('email_members', 'm')->extend('PagerDefault');
						$query
						  ->join('email_groups', 'g', 'g.gid = m.gid');
						$query
						  ->fields('m', array('email', 'name'))
								->fields('g', array('gtitle'))
								->condition('m.gid', $gid)
								->limit(30);

				  $results = $query->execute();


						foreach ($results as $row) {
								$rows[] = array(l($row->name, 'mailto:' . $row->email), $row->email);
								$gtitle[1] = $row->gtitle;
						}

						$build['table'] = array(
								'#theme' => 'table',
								'#prefix' => '<h3>' . t('Members in Group <em>@group</em>', array('@group' => $gtitle[1])) . '</h3>',
								'#header' => array('Name', 'Email'),
								'#description' => t('Clicking on a name opens your default mail client to send that individual member an email.'),
								'#rows' => $rows,
								'#empty' => t('There are no emails in this table'),
								'#suffix' => theme('pager'),
						);

  $build['download'] = array(
    '#type' => 'markup',
      '#prefix' => '<p>',
      '#suffix' => '</p>',
    '#markup' => t('Download the list in .csv format: ') . l('Email List', 'box_office/admin/orders/download', array('query' => array('type' => 'email_group', 'gid' => $gid))),
  );

						return $build;
				}
				else {
				  drupal_goto('email/groups/');
				}
}
