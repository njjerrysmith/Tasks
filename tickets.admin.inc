<?php // $Id$

/**
 * @file
 * This file contains all the functions for the Box Office Admin pages.
 */

/**
 * Page callback for Box Office Admin page
 *
 * This function builds a table with ticket
 * information and returns it in the theme function
 * styled as a table.
 *
 * @return
 *	$build;
 */
function tasks_tickets_default() {
	$build = '<div id="intro">' . t('The table rendered below is of all the tickets in the database.') . '</div>';
	$query = db_select('tickets', 't');
	$query->join('seasons', 's', 't.sid = s.season_id');
	$query
		->fields('t', array('tid', 'title', 'price'))
		->fields('s', array('season_id', 'season_desc'))
		->groupBy('tid')
		->orderBy('season_id', 'DESC');

	$result = $query->execute();

	$output = '';
	$rows = array();
	foreach ($result as $entry) {
		// Sanitize the data before handing it off to the theme layer.
		$rows[] = drupal_map_assoc(array($entry->season_desc, $entry->title, $entry->price), 'check_plain');
	}

	// Make a table for them.
	$header = array(t('Ticket Season'), t('Ticket Title'), t('Ticket Price'));
	$output .= theme('table', array('header' => $header, 'rows' => $rows));

	$build .= $output;

	return $build;
}


/**
 * Validation function for maintating tickets
 *
 * This funciton is a universal validator for the
 * ticket management functions. Validates the title
 * (if it is not on the update page) to make sure it
 * is not in the database, and the price to make sure
 * it is in the format $##.##.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return
 *	form_set_error()
 */
function tasks_ticket_form_validation ($form, &$form_state) {
	// Title Validation
	if(!$_POST['form_id'] = 'tasks-tickets-update-form') {
		$query = db_select('tickets', 't');

		$query
			->fields('t', array('title'))
			->condition('title', $form_state['values']['title']);

		$results = $query->execute();

		$numrows = $results->rowCount();

		print_r($numrows);

		if ($numrows >= 1) {
			form_set_error('title', t('Ticket title "@title" is already in use. Please rename your ticket.', array('@title' => $form_state['values']['title'])));
		}
	}

	// Price Validation
	if(!preg_match('/[$][0-9]{1,3}(?:,?[0-9]{3})*(?:\.[0-9]{2})?/', $form_state['values']['price'])) {
		form_set_error('price', t('"@price" was not entered in the corrent format. The corret format is $##.##', array('@price' => $form_state['values']['price'])));
	}
}


/**
 * Page callback for the Add Tickets Page
 *
 * This function returns a render array with an
 * introduction and the form for the page.
 *
 * @return
 *	array();
 */
function tasks_tickets_add() {
	return array(
		'intro' => array(
			'#prefix' => '<div style="intro">',
			'#suffix' => '</div>',
			'#markup' => t('Use this form to add a ticket to be "sold" on the LTC website. All information is required. Selling a ticket requests personal information from the user and then prompts them to complete the purchase by mailing a check to the LTC office.'),
		),
		'form' => drupal_get_form('tasks_tickets_add_form'),
	);
}


/**
 * Form for Add Ticket page
 *
 * This form returns a textfield for the title of
 * the ticket, a textfield for the price of the
 * ticket (validated in the format $##.##), and
 * a dropdown to associate the ticket with a season.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return
 *	$form
 */
function tasks_tickets_add_form($form, &$form_state) {
	$form = array();

	$form['title'] = array(
		'#title' => t('Title of Ticket'),
		'#type' => 'textfield',
		'#required' => TRUE,
	);

	$form['price'] = array(
		'#title' => t('Price of Ticket'),
		'#type' => 'textfield',
		'#description' => t('Format: $##.##'),
		'#required' => TRUE,
	);

	$form['season'] = array(
		'#title' => t('Season'),
		'#type' => 'select',
		'#options' => _getSeasons(),
		'#default_value' => variable_get('season_current'),
		'#required' => TRUE,
	);

	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Create Ticket'),
	);

	$form['#validate'][] = 'tasks_ticket_form_validation';

	return $form;
}


/**
 * Submit handler for Add Ticket form
 *
 * After validation, this function inserts
 * the information into the tickets table.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return
 *	drupal_set_message()
 */
function tasks_tickets_add_form_submit($form, &$form_state) {
	$query = db_insert('tickets')
		->fields(array(
			'tid' => NULL,
			'title' => $form_state['values']['title'],
			'price' => $form_state['values']['price'],
			'sid' => $form_state['values']['season'],
		))
		->execute();

	drupal_set_message(t('@ticket created.',
		array('@ticket' => $form_state['values']['title'])));
}


/**
 * Read from the database using a filter array.
 *
 * This function returns a filter array of all the entries
 * in the tickets table. Used to populate dropdowns on the update
 * and delete pages.
 *
 * @param $entry
 *   An array containing all the fields used to search the entries in the table.
 *
 * @return
 *   An object containing the loaded entries if found.
 */
function tasks_ticket_entry_load($entry = array()) {
  $select = db_select('tickets', 't');
	$select->join('seasons', 's');

	$select
		->fields('t', array('tid', 'title', 'price', 'sid'))
		->fields('s', array('season_id', 'season_desc'))
		->orderBy('season_id', 'DESC')
		->orderBy('tid', 'DESC');


  // Add each field and value as a condition to this query.
  foreach ($entry as $field => $value) {
    $select->condition($field, $value);
  }
  // Return the result in object format.
  return $select->execute()->fetchAll();
}


/**
 * AJAX callback to return information to populate data based
 * on selection.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return
 *	$form
 */
function tasks_tickets_update_ajax_callback($form, $form_state) {
  $entry = $form_state['entries'][$form_state['values']['tid']];

  // Setting the #value of items is the only way I was able to figure out
  // to get replaced defaults on these items. #default_value will not do it
  // and shouldn't.

  foreach (array('tid', 'sid', 'title', 'price') as $item) {
    $form[$item]['#value'] = $entry->$item;
  }
  return $form;
}


/**
 * Page callback for the Update Tickets Page
 *
 * This function returns a render array with an
 * introduction and the form for the page.
 *
 * @return
 *	array();
 */
function tasks_tickets_update() {
	return array(
		'intro' => array(
			'#prefix' => '<div id="intro">',
			'#suffix' => '</div>',
			'#markup' => t('This form updates a ticket currently in the database. By selecting a ticket in the dropdown, it\'s information will automatically be populated in the rest of the text fields. Replacing it and clicking submit will complete the update process.'),
		),
		'form' => drupal_get_form('tasks_tickets_update_form'),
	);
}


/**
 * Form for Update Tickets page
 *
 * This form returns a dropdown of current tickets
 * which is ajax enabled to load the other fields
 * with their content, a textfield for the title of
 * the ticket, a textfield for the price of the
 * ticket (validated in the format $##.##), and
 * a dropdown to associate the ticket with a season.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return
 *	$form
 */
function tasks_tickets_update_form($form, &$form_state) {
	$form  = array(
    '#prefix' => '<div id="updateform">',
    '#suffix' => '</div>',
  );

	$entries = tasks_ticket_entry_load();

  	$keyed_entries = array();
  		if (empty($entries)) {
    		$form['no_values'] = array(
      			'#value' => t('No entries exist.'),
    		);
    	return $form;
  		}

  foreach ($entries as $entry) {
    $options[$entry->tid] = t('@tid: @title', array('@tid' => $entry->tid, '@title' => $entry->title));
		$seasons[$entry->season_id] = t('@season_id: @season_desc', array('@season_desc' => $entry->season_desc, '@season_id' => $entry->season_id));
    $keyed_entries[$entry->tid] = $entry;
  }

  $form_state['entries'] = $keyed_entries;

  $form['tid'] = array(
  	'#title' => t('Original Ticket'),
		'#description' => t('Select a ticket to update'),
    '#type' => 'select',
    '#options' => $options,
		'#default_value' => $entry->tid,
		'#ajax' => array(
      'wrapper' => 'updateform',
      'callback' => 'tasks_tickets_update_ajax_callback',
    ),
		'#required' => TRUE,
  );

	$form['title'] = array(
		'#title' => t('Title of Ticket'),
		'#type' => 'textfield',
		'#description' => t('Only alphanumeric characters allowed'),
		'#required' => TRUE,
		'#default_value' => $entry->title,
	);

	$form['price'] = array(
		'#title' => t('Price of Ticket'),
		'#type' => 'textfield',
		'#required' => TRUE,
		'#default_value' => $entry->price,
	);

	$form['season'] = array(
		'#title' => t('Season'),
		'#type' => 'select',
		'#options' => $seasons,
		'#required' => TRUE,
		'#default_value' => $entry->sid,
	);

	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Update Ticket'),
	);

	$form['#validate'][] = 'tasks_ticket_form_validation';

	return $form;
}


/**
 * Submit handler for Update Tickets form
 *
 * After validation, this function updates
 * the information into the tickets table
 * where the tid is equal to the one set in
 * the form by the user.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return
 *	drupal_set_message()
 */
function tasks_tickets_update_form_submit ($form, &$form_state) {
  $nid = db_update('tickets')
  	->fields(array(
	  	'title' => $form_state['values']['title'],
			'price' => $form_state['values']['price'],
			'sid' => $form_state['values']['season'],
	  ))
		->condition('tid', $form_state['values']['tid'])
		->execute();

  drupal_set_message(t('@ticket sucessfully updated.', array('@ticket' => $form_state['values']['title'])));
}


/**
 * Page callback for the Delete Tickets Page
 *
 * This function returns a render array with an
 * introduction and the form for the page.
 *
 * @return
 *	array();
 */
function tasks_tickets_delete() {
	return array(
		'intro' => array(
			'#prefix' => '<div class="intro">',
			'#suffix' => '</div>',
			'#markup' => t('Use this page to delete a ticket from the ticket sales database.') . ' <i>' . t('Be careful, this action can not be undone.') . '</i>',
		),
		'form' => drupal_get_form('tasks_tickets_delete_form'),
	);
}


/**
 * Form for Delete Tickets page
 *
 * This form returns a dropdown of current tickets
 * where one is selected to be deleted.
 *
 * TODO: Fix confirmation step.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return
 *	$form
 */
function tasks_tickets_delete_form() {
	$entries = tasks_ticket_entry_load();

	$keyed_entries = array();
		if (empty($entries)) {
			$form['no_values'] = array(
					'#value' => t("No entries exist in the seasons table."),
			);
		return $form;
		}

  foreach ($entries as $entry) {
    $options[$entry->tid] = t("@tid: @title", array('@tid' => $entry->tid, '@title' => $entry->title));
    $keyed_entries[$entry->tid] = $entry;
  }

  $default_entry = !empty($form_state['values']['edit']['title']) ? $keyed_entries[$form_state['values']['edit']['season_id']] : $entries[0];

  $form_state['entries'] = $keyed_entries;

  $form['tid'] = array(
  	'#title' => t('Ticket'),
		'#description' => t('Select a ticket to delete'),
    '#type' => 'select',
    '#options' => $options,
		'#required' => TRUE,
  );

	$form['submit'] = array(
		'#value' => t('Delete Ticket'),
		'#type' => 'submit',
	);

	return $form;
}

/**
 * Submit handler for Ticket delete form
 *
 * After validation, this function delete
 * the information in the tickets table
 * where the tid is equal to the one set in
 * the form by the user. There is also a built
 * in error catching method where the error is
 * stored with watchdog, and a page error returned.
 *
 * TODO: Implement form_confirm();
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return
 *	drupal_set_message()
 */
function tasks_tickets_delete_form_submit($form, &$form_state) {
  try {
		$query = db_delete('tickets');
		$query
			->condition('tid', $form_state['values']['tid']);

		if(!$query->execute()) {
    	throw new Exception('DB Delete Fail.');
    }
	}

	catch (Execption $e) {
		watchdog('tasks-tickets', t('@e', array('@e' => $e->getMessage())), $severity = WATCHDOG_ERROR, $_SEVER['PHP_SELF']);
		drupal_set_message('Error', 'error');
	}

	drupal_set_message('Ticket @tid has been deleted', array('@tid' => $form_state['values']['tid']));
}


/**
 * Page callback for Box Office Introduction Page
 *
 * This function returns a render array with an
 * introduction and the form for the page.
 *
 * @return
 *	array();
 */
function tasks_boxoffice_intro() {
	return array(
		'intro' => array(
			'#prefix' => '<div style="intro">',
			'#suffix' => '</div>',
			'#markup' => t('Use this form to change the introduction on the ') . l('Box Office', '/box_office') . t(' page.'),
		),
		'form' => drupal_get_form('tasks_boxoffice_intro_form'),
	);
}


/**
 * Form for Box Office Introduction page
 *
 * This form returns a text format enabled
 * version of a text area for updating the page
 * heading on the Box Office homepage.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return
 *	system_settings_form()
 */
function tasks_boxoffice_intro_form($form, &$form_state) {
	$form = array();

	$default = variable_get('boxoffice_intro');
	$default_text = $default['value'];
	$default_format = $default['format'];

	$form['boxoffice_intro'] = array(
	 '#type' => 'text_format',
		'#title' => t('Box Office Introduction Paragraph'),
		'#description' => t('Use this text area for an introduciton to the box office, and when show tickets are available, paste the button here.') . ' <em>' . t('Use no heading larger then H3') .'</em>',
		'#resizable' => TRUE,
		'#cols' => 120,
  '#rows' => 18,
		'#default_value' => $default_text,
		'#format' => $default_format,
		'#required' => TRUE,
	);

 $select = db_select('seasons', 's');
 $select
   ->fields('s', array('season_id', 'season_desc'));

 $results = $select->execute();

 foreach ($results as $row) {
   $opt_desc[$row->season_desc] = t('@desc', array('@desc' => $row->season_desc));
   $opt_id[$row->season_id] = t('@desc', array('@desc' => $row->season_desc));
 }

 $form['sale'] = array(
  '#type' => 'fieldset',
  '#title' => t('Season Subscription For Sale'),
 );

 $form['sale']['tasks_tickets_season_for_sale'] = array(
  '#type' => 'select',
  '#options' => $opt_desc,
  '#title' => t('Season Subscription for Sale'),
  '#default_value' => variable_get('tasks_tickets_season_for_sale'),
 );
 $form['sale']['tasks_tickets_season_for_sale_id'] = array(
  '#type' => 'select',
  '#options' => $opt_id,
  '#title' => t('Season Subscription for Sale'),
  '#default_value' => variable_get('tasks_tickets_season_for_sale_id'),
 );
 $form['sale']['expl'] = array(
  '#type' => 'item',
  '#value' => t('<strong>Make sure</strong> that both of these values are always equal.'),
 );

 $form['boxoffice_seasontickets'] = array(
		'#title' => t('Season Tickets Status'),
		'#description' => t('Turn Season Tickets sales on or off.'),
		'#type' => 'select',
		'#options' => array(
				1 => t('On'),
				2 => t('Off'),
		),
		'#default_value' => variable_get('boxoffice_seasontickets'),
	);

	return system_settings_form($form);
}

/**
 * Page callback for the deafult ticket orders page
 */
function tasks_boxoffice_suborders() {
  $build = array(
    'intro' => array(
      '#type' => 'markup',
      '#markup' => t('Use this page to change the payment status of an order.'),
      '#prefix' => '<p>',
      '#suffix' => '</p>',
    ),
  );

  $query = db_select('tickets_orders', 't');
  $query
    ->fields('t', array('unique_id', 'orderid', 'fname', 'lname', 'total', 'paid'));
  $results = $query->execute();

  foreach ($results as $row) {
    $name = $row->fname . ' ' . $row->lname;

    if($row->paid == 1){
      $paid = t('Yes');
    }

    if($row->paid == 0){
      $paid = t('No');
    }

    $rows = array();

    $rows[] = array_map('check_plain', array($row->orderid, $name, t('$@price.00', array('@price' =>$row->total)), $paid));
  }

  $header = array(t('Order ID'), t('Name'), t('Order Total'), t('Payment Status'));
  $build['Payment-List'] = array(
    '#rows' => $rows,
    '#header' => $header,
    '#theme' => 'table',
    '#attributes' => array('class' => array('board-list, involve-list')),
    '#empty' => t('There are no orders to display.'),
  );

  $build['form'] = drupal_get_form('tasks_boxoffice_suborder_form');

  return $build;
}

/**
 * Returns a form to mark ticket orders as paid.
 */
function _getOrders() {
  $query = db_select('tickets_orders', 't');
  $query
    ->fields('t', array('unique_id', 'orderid'));

  $results = $query->execute();

  $rows = array();

  foreach ($results as $row) {
    $rows[$row->unique_id] = $row->orderid;
  }

  return $rows;
}

function tasks_boxoffice_suborder_form($form, &$form_state) {
  $form = array();
  $form['order_id'] = array(
    '#title' => t('Order Number'),
    '#type' => 'select',
    '#required' => TRUE,
    '#options' => _getOrders(),
    '#prefix' => '<table><tr><th style="text-align: center">',
    '#suffix' => '</th>',
  );

  $form['paid'] = array(
    '#type' => 'select',
    '#title' => t('Payment Status'),
    '#options' => array(
      '0' => t('No'),
      '1' => t('Yes'),
    ),
    '#prefix' => '<th style="text-align: center">',
    '#suffix' => '</th>',
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update Order'),
    '#prefix' => '<th style="text-align: center"><br />',
    '#suffix' => '</th></tr></table>',
  );

  return $form;
}

function tasks_boxoffice_suborder_form_submit($form, &$form_state) {
  $txn = db_transaction();
  try {
    $query = db_update('tickets_orders')
    ->fields(array(
      'paid' => $form_state['values']['paid'],
    ))
    ->condition('unique_id', $form_state['values']['order_id'])
    ->execute();

    drupal_set_message(t('Order updated @value', array('@value' => $form_state['values']['paid'])));
  }

  catch (Exception $e) {
      $txn->rollback();
    		watchdog('php', $e);
      drupal_set_message(t('There was an error updating the payment status. Please contact the webmaster.'), 'error');
  }
}

/**
 * Page callback for the ticket mailing list page
 */
function tasks_boxoffice_submail() {
  $build = array(
    'intro' => array(
      '#type' => 'markup',
      '#markup' => t('Use this page to print a mailing list for orders that have been marked as paid.'),
      '#prefix' => '<p>',
      '#suffix' => '</p>',
    ),
  );

  $query = db_select('tickets_orders', 't');
  $query
    ->fields('t', array('unique_id', 'orderid', 'fname', 'lname', 'email', 'address', 'city', 'state', 'zip'))
    ->condition('paid', '1')
    ->orderBy('unique_id', 'ASC');
  $results = $query->execute();

  foreach ($results as $row) {
    $name = $row->fname . ' ' . $row->lname;

    if($row->paid = 1){
      $paid[$row->unique_id] = t('Yes');
    }

    if($row->paid = 0){
      $paid[$row->unique_id] = t('No');
    }

    $rows[] = array_map('check_plain', array($row->orderid, $name, $row->email, $row->address, $row->city, $row->state, $row->zip));
  }

  $header = array(t('Order ID'), t('Name'), t('Email'), t('Address'), t('City'), t('State'), t('Zip'));
  $build['Payment-List'] = array(
    '#rows' => $rows,
    '#header' => $header,
    '#theme' => 'table',
    '#empty' => t('There are no orders to display.'),
  );

  $build['download'] = array(
    '#type' => 'markup',
    '#prefix' => '<p>',
    '#suffix' => '</p>',
    '#markup' => t('Download the list in .csv format: ') . l('Mailing List', 'box_office/admin/orders/download', array('query' => array('type' => 'mail'))),
  );

  return $build;
}

/**
 * Page callback for the deafult ticket seat preference page
 */
function tasks_boxoffice_subseat() {
  $build = array(
    'intro' => array(
      '#type' => 'markup',
      '#markup' => t('Use this page to print a seating list for orders that have been marked as paid.'),
      '#prefix' => '<p>',
      '#suffix' => '</p>',
    ),
  );

  $query = db_select('tickets_orders', 't');
  $query
    ->fields('t', array('unique_id', 'orderid', 'lname', 'night', 'preference', 'qty'))
    ->condition('paid', '1')
    ->orderBy('nightnum', 'ASC');
  $results = $query->execute();

  foreach ($results as $row) {

    $rows[] = array_map('check_plain', array($row->orderid, $row->lname, $row->night, $row->preference, $row->qty));
  }

  $header = array(t('Order ID'), t('Reservation Name'), t('Night'), t('Preference'), t('Qty'));
  $build['Payment-List'] = array(
    '#rows' => $rows,
    '#header' => $header,
    '#theme' => 'table',
    '#empty' => t('There are no orders to display.'),
  );

  $build['download'] = array(
    '#type' => 'markup',
      '#prefix' => '<p>',
      '#suffix' => '</p>',
    '#markup' => t('Download the list in .csv format: ') . l('Seating List', 'box_office/admin/orders/download', array('query' => array('type' => 'seat'))),
  );

  return $build;
}

function tasks_tickets_csv($type = NULL) {
  $type = $_GET['type'];
  // Set Headers
  drupal_add_http_header('Content-type', 'text/csv');
  if ($type =='mail') {
    drupal_add_http_header('Content-Disposition', 'attachment; filename=SeasonTickets-Mailing-'.date("Ymd").'.csv');
  }

  if ($type == 'seat') {
    drupal_add_http_header('Content-Disposition', 'attachment; filename=SeasonTickets-Seating-'.date("Ymd").'.csv');
  }

		if ($type == 'email_group') {
    drupal_add_http_header('Content-Disposition', 'attachment; filename=EmailList-'.date("Ymd").'.csv');
		}
  drupal_add_http_header("Pragma", "no-cache");
  drupal_add_http_header("Expires", "0");

		if ($type == 'email_group') {
			$query = db_select('email_members', 'm');
			$query
				 ->join('email_groups', 'g', 'g.gid = m.gid');
			$query
			  ->condition('m.gid', $_GET['gid'])
					->fields('m', array('name', 'email'))
					->orderby('eid', 'ASC');

			$results = $query->execute();

			 // Open array
    $rows = array();
    print("Name, Email\n");
    foreach ($results as $row) {
      print("{$row->name},");
      print("{$row->email}\n");
    }
    print "\n";
  }

  if ($type == 'seat') {
    // Operate Query
    $query = db_select('tickets_orders', 't');
    $query
      ->fields('t', array('unique_id', 'orderid', 'lname', 'night', 'preference', 'qty'))
      ->condition('paid', '1')
      ->orderBy('nightnum', 'ASC');
    $results = $query->execute();

    // Open array
    $rows = array();
    print("Last Name, Night, Qty, Preference, OrderID\n");
    foreach ($results as $row) {
      $pref = check_plain($row->preference);
      if (preg_match("/,/i", $pref)) {
        $pref = strstr($pref, ',', true);
      }
      print("{$row->lname},");
      print("{$row->night},");
      print("{$row->qty},");
      print("{$pref},");
      print("{$row->orderid}\n");
    }
    print "\n";
  }

  if ($type == 'mail') {
    // Operate Query
    $query = db_select('tickets_orders', 't');
    $query
      ->fields('t', array('unique_id', 'orderid', 'fname', 'lname', 'night', 'email', 'address', 'city', 'state', 'zip',))
      ->condition('paid', '1')
      ->orderBy('nightnum', 'ASC')
      ->orderBy('lname', 'ASC')
      ->orderBy('fname', 'ASC');
    $results = $query->execute();

    // Open array
    $rows = array();
    print("First Name, Last Name, Email, Address, City, State, Zip Night, OrderID\n");
    foreach ($results as $row) {
      print("{$row->fname},");
      print("{$row->lname},");
      print("{$row->email},");
      print("{$row->address},");
      print("{$row->city},");
      print("{$row->state},");
      print("{$row->zip},");
      print("{$row->orderid}\n");
    }
    print "\n";
  }
}
function tasks_boxoffice_delete() {
  $build = array();

  $build['intro'] =  array(
    '#type' => 'markup',
    '#markup' => t('Use this page to delete an order from the database. <br /><strong>Warning: This operation cannot be undone</strong>'),
  );
  $build['form']  =  drupal_get_form('tasks_boxoffice_delete_form');

  return $build;
}

function tasks_boxoffice_delete_form($form, &$form_state) {
  $query = db_select('tickets_orders', 't');
  $query
    ->fields('t', array('unique_id', 'orderid', 'fname', 'lname'))
    ->orderBy('unique_id', 'ASC');

  $results = $query->execute();

  $options = array();

  foreach ($results as $row) {
    $options[$row->unique_id] = t('@fname @lname (@orderid)', array('@fname' => $row->fname, '@lname' => $row->lname, '@orderid' => $row->orderid));
  }

  $form = array();

  $form['order_id'] = array(
    '#title' => t('Order Number'),
    '#type' => 'select',
    '#required' => TRUE,
    '#options' => $options,
    '#empty' => t('There are no orders to delete.')
  );

  $form['reqexpl'] = array(
    '#markup' => t('Elements with an astrick (') . '<span class="form-required" title="This field is required.">' . t('*') . '</span>' . t(') are required.'),
    '#prefix' => '<p>',
    '#suffix' => '</p>',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete Order'),
  );

  return $form;
}

function tasks_boxoffice_delete_form_submit($form, &$form_state) {
$txn = db_transaction();
  try {
    $query = db_delete('tickets_orders');
    $query
      ->condition('unique_id', $form_state['values']['order_id']);

    $results = $query->execute();

    drupal_set_message(t('Record <em>@record</em> removed from database', array('@record' => $form_state['values']['order_id'])));
  }
  catch (Exception $e) {
    $txn->rollback();
    watchdog('php', $e);
    drupal_set_message(t('There was an error processing this request. Contact the webmaster'), 'error');
  }
}