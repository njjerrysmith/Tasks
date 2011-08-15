<?php

/**
 * @file
 * This is page that processes the ticket functions
 * TODO:
 * 	validate functions for each form
 */

/**
 * Returns the page content for the admin page
 *
 * This function is the render array for the
 * admin page functions. Functions are: ticket
 * sales status.
 *
 * @return array()
 */
function tasks_tickets_admin() {
	return array(
			'intro' => array(
					'#prefix' => '<div id="intro">',
					'#suffix' => '</div>',
					'#markup' => t('This page presents options for tickets function of the tasks module (sub-mod: Box Office).')
			),
			'form' => drupal_get_form('tasks_tickets_admin_form'),
		);
}


/**
 * Creates the tickets admin form
 *
 * This function creates the form for the
 * admin tickets page. Options include turning
 * ticket sales on or off.
 *
 * @param $form
 *  Structured array containing elements and properties
 *		of the form
 * @param &$form_state
 *		Structured array containing form data and information
 *
 *	@return system_settings_form()
 */
function tasks_tickets_admin_form($form, &$form_state) {
	$form = array();

	$form['boxoffice_seasontickets'] = array(
		'#title' => t('Season Tickets Status'),
		'#description' => t('Turn Season Tickets sales on or off.'),
		'#type' => 'select',
		'#options' => array(
				1 => t('On'),
				2 => t('Off'),
		),
		'#default_value' => variable_get('seasonticket_status'),
	);

  return system_settings_form($form, TRUE);
}


/**
 * Creates the Box Office Homepage
 *
 * This function runs through several
 * options and creates the Box Office
 * Homepage. Irregardless of settings
 * the variable boxoffice_intro is
 * rendered, then if ticket sales are
 * turned on, a table with the currently
 * avaliable tickets is rendered. If ticket
 * sales are off then a message to that
 * effect is rendered.
 *
 * @return
 * 	$output;
 */
function tasks_tickets_hello() {
	$output = '<h2>' . t('Welcome to the Box Office') . '</h2>';
	$output .= variable_get('boxoffice_intro[\'value\']', t('Welcome to the box office page. Here you are presented options to purchase tickets for our shows.'));

	if(variable_get('boxoffice_seasontickets') == 1) {
		$output .= '<h2>' . t('Ticket Sales') . '</h2>';

		$query = db_select('tickets', 't');
		$query->join('seasons', 's', 't.sid = s.season_id');
		$query
			->fields('t', array('tid', 'title', 'price'))
			->fields('s', array('season_id', 'season_desc'))
			->groupBy('tid')
			->orderBy('season_id', 'DESC');

		$result = $query->execute();

		$output .= '';
		$rows = array();
    foreach ($result as $entry) {
      // Sanitize the data before handing it off to the theme layer.
      $rows[] = drupal_map_assoc(array($entry->season_desc, $entry->title, $entry->price), 'check_plain');
    }

    // Make a table for them.
    $header = array(t('Season'), t('Ticket Title'), t('Ticket Price'));
    $output .= theme('table', array('header' => $header, 'rows' => $rows));

		$output .= l(t('Purchase Tickets'), 'box_office/purchase', array('attributes' => array('class' => array('button'))));

		return $output;
}

	if(variable_get('boxoffice_seasontickets') == 2) {
		$output .= '<h2>' . t('Ticket Sales') . '</h2>';
		$output .= t('Sorry. Tickets are currently not being sold. If you have questions feel free to contact ' . l(t('info@rultc.org'), 'mailto:info@rultc.org') . '.');

		return $output;
	}

	else {
		$output = t('Error determining season ticket status. Contact webmaster.');
		print_r(variable_get('boxoffice_seasontickets'));
		return $output;
	}
}



/**
 * Returns render array
 *
 * Function returns render array, if
 * variable 'boxoffice_seasontickets' is
 * set to 'off' drupal_goto is called, else
 * a build array is returned.
 *
 * @return drupal_goto;
 * @return $build;
 */
function tasks_tickets_purchase_page_one() {
  if(variable_get('boxoffice_seasontickets') == 2) {
		  drupal_goto('/box_office');
	 }
	 else {
	  $build = array(
				'header_text' => array(
      '#type' => 'markup',
      '#markup' => '<p>' . t('Purchase Hello (convert to vairable, text on admin)') . '</p>',
    ),
    'form' => drupal_get_form('tasks_tickets_page_one_form'),
  );
  return $build;
	}
}

/**
 * Returns form array for page 1
 *
 * Function returns an array of form object to
 * purchase tickets if page is supposed to be
 * first page, otherwise redirects to approriate
 * page (2 or 3). Fields are set inside a fieldset
 * titled by season, each ticket returns the following fields:
 * title (item), ticket_price_disp (item), ticket_price_item
 * (value), qty (select), and atc (button). ATC (add to cart)
 * is an ajax enabled button to add the item to the cart.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return $form;
 */
function tasks_tickets_page_one_form($form, &$form_state) {
	// Display page 2 if $form_state['page_num'] == 2
  if (!empty($form_state['page_num']) && $form_state['page_num'] == 2) {
    return tasks_tickets_page_two($form, $form_state);
  }

	// Display page 3 if $form_state['page_num'] == 3
  if (!empty($form_state['page_num']) && $form_state['page_num'] == 3) {
    return tasks_tickets_page_three_form($form, $form_state);
  }

 // Otherwise we build page 1.
 $form_state['page_num'] = 1;

	$form = array();

	$query = db_select('tickets', 't');
	$query->join('seasons', 's', 't.sid = s.season_id');
	$query
		->fields('t', array('tid', 'title', 'price', 'sid'))
		->fields('s', array('season_desc', 'season_id'))
		->orderBy('t.sid', 'DESC')
		->groupBy('t.tid');

	$results = $query->execute();

	foreach ($results as $r) {

			$form[$r->sid] = array(
				'#type' => 'fieldset',
				'#title' => 'Season: ' . $r->season_desc,
				'#collapsible' => FALSE,
			);

			$form[$r->sid]['title'] = array(
				'#type' => 'item',
				'#description' => $r->title,
				'#title' => t('Ticket'),
				'#prefix' => '<div style="width: 29%; float:left">',
				'#suffix' => '</div>',
			);
			$from[$r->sid]['tixdesc'] = array(
				'#type' => 'value',
				'#value' => $r->title,
			);
			$form[$r->sid]['tid'] = array(
				'#type' => 'value',
				'#value' => $r->tid,
			);
			$form[$r->sid]['price_disp'] = array(
				'#type' => 'item',
				'#title' => t('Ticket Price'),
				'#description' => $r->price,
				'#prefix' => '<div style="width: 19%; float:left">',
				'#suffix' => '</div>',
			);
			$form[$r->sid]['price_int'] = array(
				'#type' => 'value',
				'#value' => substr($r->price, 1),
			);
			$form[$r->sid]['qty'] = array(
				'#type' =>  'select',
				'#title' => t('Quantity') . '<span class="form-required" title="This field is required."> ' . t('*') . '</span>',
				'#options' => array(
					'sel' => t('- Select -'),
					'1' => t('1'),
					'2' => t('2'),
					'3' => t('3'),
					'4' => t('4'),
					'5' => t('5'),
					'6' => t('6'),
					'7' => t('7'),
					'8' => t('8'),
					'9' => t('9'),
					'10' => t('10'),
				),
				'#default_value' => '0',
				'#prefix' => '<div style="width: 19%; float:left">',
				'#suffix' => '</div>',
			);
			/*$form[$r->sid]['atc'] = array(
					'#type' => 'button',
					'#value' => t('Add to Cart'),
					'#prefix' => '<br />',
					'#ajax' => array(
							'wrappper' => 'total',
							'callback' => tasks_tickets_add_to_cart($r->tid, $form_state['values']['qty']),
					),
				);*/
	}

	/* This whole form element is rebuilt in the ajax callback (after it's built)
	$form['order_total'] = array(
			'#type' => 'item',
			'#prefix' => '<div style="width: 50%; float: left"><div id="total"><b>' . t('Order Total:') . ' </b>',
			'#suffix' => '</div></div>',
			'#markup' => "'" .
      (!empty($form_state['values']['order_total'])
      ? $form_state['values']['order_total'] : t("$0.00")) . "'",
		);*/

	$form['reqexpl'] = array(
  '#markup' => t('Elements with an astrick (') . '<span class="form-required" title="This field is required.">' . t('*') . '</span>' . t(') are required.'),
  '#prefix' => '<p>',
  '#suffix' => '</p>',
	);

	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Next'),
		//'#prefix' => '<div style="width: 50%; float: left"><br />',
		//'#suffix' => '</div><div style="clear:both"></div>',
		'#submit' => array('tasks_tickets_page_one_form_submit'),
	);

	$form['#validate'][] = 'tasks_tickets_page_one_form_validate';

	return $form;
}

/*function tasks_tickets_add_to_cart($form, &$form_state, $tid =  NULL, $qty = NULL) {
	$query = db_select('tickets', 't');
	$query
		->fields('t', array('tid', 'price'))
		->conditions('tid', $tid);

	$results = $query->execute();

	foreach($results as $row) {
		$price = substr($row->price, 1);
	}
	$tot_price = $price * $qty;

	$form_state['values']['order_total'] =  t('$@price', array('@price' => $tot_price));
}*/

function tasks_tickets_page_one_form_validate($form, &$form_state) {
	if($form_state['page_values']['1']['qty'] == 'sel') {
		form_set_error('qty', t('Field Quantity is required'));
	}
}
/**
 * Submit handeler for purchase page one
 *
 * This function simply carries the values
 * from the first page over to the second
 * page.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 */
function tasks_tickets_page_one_form_submit($form, &$form_state) {
  // Values are saved for each page.
  // to carry forward to subsequent pages in the form.
  // and we tell FAPI to rebuild the form.
  $form_state['page_values'][1] = $form_state['values'];

  if (!empty($form_state['page_values'][2])) {
    $form_state['values'] = $form_state['page_values'][2];
  }

  // When form rebuilds, it will look at this to figure which page to build.
  $form_state['page_num'] = 2;
  $form_state['rebuild'] = TRUE;
}

function tasks_tickets_page_two($form, $form_state) {
	 $build = array(
				'header_text' => array(
      '#type' => 'markup',
      '#markup' => '<h3><em>' . t('Ordering @qty @ticket at $@price each.', array('@qty' => $form_state['values']['qty'], '@ticket' => $form_state['values']['qty'], '@price' => $form_state['values']['price_int'])) . '</em></h3>',
    ),
    'form' => drupal_get_form('tasks_tickets_page_two_form'),
  );
  return $build;
}
/**
 * Returns form array for page 2
 *
 * Function returns an array of form object to
 * purchase tickets if page is supposed to be
 * first page, otherwise redirects to approriate
 * page (2 or 3). Fields are set inside three fieldsets:
 * info, seating, donation. The info fieldset collects
 * personal information including Full Name (2 textfields),
 * email (textfield), home address (textfield), city
 * (textfield), state (select) and zip (textfield).
 * The seating fieldset collects seating information
 * including show (select), and seating preference
 * (textfield). The donation fieldset is optional and
 * contains a single textfield to collect a donation
 * amount if the purchaserer wishes to make one.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return $form;
 */
function tasks_tickets_page_two_form($form, $form_state) {

		// Display page 3 if $form_state['page_num'] == 3
  if (!empty($form_state['page_num']) && $form_state['page_num'] == 3) {
    return tasks_tickets_page_three($form, $form_state) or DIE('Hit error');
  }

	$form = array();

	$form['info'] = array(
		'#type' => 'fieldset',
		'#title' => t('Personal Information'),
	);

	$form['info']['fname'] = array(
		'#type' => 'textfield',
		'#title' => t('First Name'),
	  '#size' => 20,
    '#maxlength' => 20,
    '#required' => TRUE,
		'#prefix' => '<div><div style="width: 48%; float: left;">',
		'#suffix' => '</div>',
	);

	$form['info']['lname'] = array(
		'#type' => 'textfield',
		'#title' => t('Last Name'),
		'#descriprtion' => t('This is the name which the seats will be reserved under.'),
	 '#size' => 20,
  '#maxlength' => 20,
  '#required' => TRUE,
		'#prefix' => '<div><div style="width: 48%; float: left;">',
		'#suffix' => '</div><div style="clear: both"></div>',
	);
	$form['info']['email'] = array(
		'#type' => 'textfield',
		'#title' => t('E-Mail Address'),
    '#required' => TRUE,
		'#validate' => 'valid_email_address',
	);
	$form['info']['haddress'] = array(
		'#type' => 'textfield',
		'#title' => t('Home Address'),
    '#required' => TRUE,
	);
		$form['info']['hcity'] = array(
		'#type' => 'textfield',
		'#title' => t('Home City'),
    '#required' => TRUE,
	);
		$form['info']['hstate'] = array(
		'#type' => 'select',
		'#title' => t('Home State'),
    '#required' => TRUE,
		'#options' => array(
			'AL' => t('Alabama'),
			'AK' => t('Alaska'),
			'AZ' => t('Arizona'),
			'AR' => t('Arkansas'),
			'CA' => t('California'),
			'CO' => t('Colorado'),
			'CT' => t('Connecticut'),
			'DE' => t('Delaware'),
			'DC' => t('District Of Columbia'),
			'FL' => t('Florida'),
			'GA' => t('Georgia'),
			'HI' => t('Hawaii'),
			'ID' => t('Idaho'),
			'IL' => t('Illinois'),
			'IN' => t('Indiana'),
			'IA' => t('Iowa'),
			'KS' => t('Kansas'),
			'KY' => t('Kentucky'),
			'LA' => t('Louisiana'),
			'ME' => t('Maine'),
			'MD' => t('Maryland'),
			'MA' => t('Massachusetts'),
			'MI' => t('Michigan'),
			'MN' => t('Minnesota'),
			'MS' => t('Mississippi'),
			'MO' => t('Missouri'),
			'MT' => t('Montana'),
			'NE' => t('Nebraska'),
			'NV' => t('Nevada'),
			'NH' => t('New Hampshire'),
			'NJ' => t('New Jersey'),
			'NM' => t('New Mexico'),
			'NY' => t('New York'),
			'NC' => t('North Carolina'),
			'ND' => t('North Dakota'),
			'OH' => t('Ohio'),
			'OK' => t('Oklahoma'),
			'OR' => t('Oregon'),
			'PA' => t('Pennsylvania'),
			'RI' => t('Rhode Island'),
			'SC' => t('South Carolina'),
			'SD' => t('South Dakota'),
			'TN' => t('Tennessee'),
			'TX' => t('Texas'),
			'UT' => t('Utah'),
			'VT' => t('Vermont'),
			'VA' => t('Virginia'),
			'WA' => t('Washington'),
			'WV' => t('West Virginia'),
			'WI' => t('Wisconsin'),
			'WY' => t('Wyoming'),
		),
  '#required' => TRUE,
		'#prefix' => '<div><div style="width: 48%; float: left;">',
		'#suffix' => '</div>',
	);
		$form['info']['hzip'] = array(
		'#type' => 'textfield',
		'#title' => t('Home Zip Code'),
		'#size' => 20,
  '#maxlength' => 20,
  '#required' => TRUE,
		'#prefix' => '<div><div style="width: 48%; float: left;">',
		'#suffix' => '</div><div style="clear: both"></div>',
	);

	$form['seating'] = array(
		'#type' => 'fieldset',
		'#title' => t('Show Information'),
	);

	$form['seating']['show'] = array(
		'#type' => 'select',
		'#title' => t('Show Preference'),
    '#required' => TRUE,
		'#options' => drupal_map_assoc(array('Thursday Night', 'Friday Night', 'Saturday Matinee', 'Saturday Night', 'Sunday Matinee'), 'check_plain'),
		'#description' => t('In the event another show is added, you will be notified and given the option to change your reservation'),
	);
	$form['seating']['seat'] = array(
		'#type' => 'textfield',
		'#title' => t('Seating Preference'),
    '#required' => TRUE,
		'#description' => t('Enter a preference for seating. Example: "Middle Section, 3rd Row".'),
	);

	$form['donation'] = array(
		'#type' => 'fieldset',
		'#title' => t('Make a Donation'),
	);

	$form['donation']['amount'] = array(
		'#type' => 'textfield',
		'#title' => t('Donation Amount'),
		'#size' => 40,
		'#max_legnth' => 40,
		'#default_value' => t('$'),
		'#prefix' => '<div style="width: 100%; float:left">',
		'#suffix' => '</div>',
		'#description' => t('Enter an amount of donation in USD in the format $##.## (Note this donation <b>is</b> tax deductible).'),
	);

	$form['reqexpl'] = array(
  '#markup' => t('Elements with an astrick (') . '<span class="form-required" title="This field is required.">' . t('*') . '</span>' . t(') are required.'),
  '#prefix' => '<p>',
  '#suffix' => '</p>',
	);

	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Checkout'),
		'#submit' => array('tasks_tickets_page_two_form_submit')
	);

	return $form;
}


/**
 * Submit handeler for purchase page two
 *
 * This function simply carries the values
 * from the first page over to the second
 * page.
 *
 * @param $form
 *  Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 */
function tasks_tickets_page_two_form_submit($form, &$form_state) {
  // Values are saved for each page.
  // to carry forward to subsequent pages in the form.
  // and we tell FAPI to rebuild the form.
  $form_state['page_values'][2] = $form_state['values'];

  if (!empty($form_state['page_values'][3])) {
    $form_state['values'] = $form_state['page_values'][3];
  }

  // When form rebuilds, it will look at this to figure which page to build.
  $form_state['page_num'] = 3;
  $form_state['rebuild'] = TRUE;

}


/**
 * Returns a render array for purchase page three
 *
 * TODO: Write funciton, documentation.
 *
 * @param $form
 * Structured array containing elements and properties
 *	of the form
 * @param &$form_state
 *	Structured array containing form data and information
 *
 * @return $form;
 */
function tasks_tickets_page_three($form, &$form_state) {
  $build = array(
		  'header_text' => array(
			    '#type' => 'markup',
			    '#value' =>
					    t('Thank you @first @last for your interest in @ticketdescription. To complete your order you will need to print our the pdf file (requires', array('@firstname' => $form_state['page_values']['2']['fname'], '@lastname' => $form_state['page_values']['2']['lname'], '@ticketdescription' => $form_state['page_values']['1']['tixdesc'])) .
					    l('Adobe Reader', 'http://get.adobe.com/reader/') .
					    t(') to download. For posterity a copy has been emailed to the address that you have provided.'),
		   ),
		  'form' => drupal_get_form('tasks_tickets_admin_form'),
		  );

		return $build;
}

function tasks_tickets_page_three_form($form, &$form_state) {
	$form = array();
	$form['complete'] = array(
		'#type' => 'checkbox',
		'#title' => t('Completed'),
		'#description' => t('Check this box once you have downloaded the pdf.'),
		'#required' => TRUE,
	);

	$form['reqexpl'] = array(
  '#markup' => t('Elements with an astrick (') . '<span class="form-required" title="This field is required.">' . t('*') . '</span>' . t(') are required.'),
  '#prefix' => '<p>',
  '#suffix' => '</p>',
 );

	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Complete Purchase'),
	);

	return $form;
}