<?php

/**
 * @file
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
 *  of the form
 * @param &$form_state
 *  Structured array containing form data and information
 *
 * @return system_settings_form()
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
 *  $output;
 */
function tasks_tickets_hello() {
 $output = '<h2>' . t('Welcome to the Box Office') . '</h2>';

	$var=(variable_get('boxoffice_intro'));
	$output .= $var['value'];

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

function tasks_tickets_order() {
 return array(
   'intro' => array(
     '#prefix' => '<div id="intro">',
     '#suffix' => '</div>',
     '#markup' => t('<strong>Thank you for your interest in our season ticket package.</strong><p> Completing this form is the first of two steps required in obtaining your season tickets. Once this form is complete you will
                    be directed to a new page which will prompt you with further instructions.</p>')
   ),
   'form' => drupal_get_form('tasks_tickets_order_form'),
  );
}

function tasks_tickets_order_form($form, &$form_state) {
 $form = array();

 $form['ticket'] = array(
  '#type' => 'fieldset',
  '#title' => t('Ticket Information'),
 );

 $form['ticket']['title'] = array(
    '#type' => 'item',
    '#description' => t('@season Season Ticket Subscription', array('@season' => variable_get('tasks_tickets_season_for_sale'))),
    '#title' => t('Ticket'),
    '#prefix' => '<div style="width: 33%; float:left">',
    '#suffix' => '</div>',
   );
  $from['ticket']['sid'] = array(
   '#type' => 'hidden',
   '#value' => t('14'),
  );
   $form['ticket']['price_disp'] = array(
    '#type' => 'item',
    '#title' => t('Ticket Price'),
    '#description' => t('$35.00'),
    '#prefix' => '<div style="width: 19%; float:left">',
    '#suffix' => '</div>',
   );
   $form['ticket']['price_int'] = array(
    '#type' => 'value',
    '#value' => '35.00',
   );
   $form['ticket']['qty'] = array(
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
    '#prefix' => '<div style="width: auto; float: left">',
    '#suffix' => '</div>',
   );

   $form['info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Personal Information'),
   );

   $form['info']['fname'] = array(
     '#type' => 'textfield',
     '#title' => t('First Name'),
     '#size' => 30,
     '#maxlength' => 30,
     '#required' => TRUE,
     '#prefix' => '<div><div style="width: 48%; float: left;">',
     '#suffix' => '</div>',
   );

 $form['info']['lname'] = array(
  '#type' => 'textfield',
  '#title' => t('Last Name'),
  '#descriprtion' => t('This is the name which the seats will be reserved under.'),
  '#size' => 30,
  '#maxlength' => 30,
  '#required' => TRUE,
  '#prefix' => '<div><div style="width: 48%; float: left;">',
  '#suffix' => '</div><div style="clear: both"></div>',
 );
 $form['info']['email'] = array(
  '#type' => 'textfield',
  '#title' => t('E-Mail Address'),
  '#required' => TRUE,
  '#element_validate' => 'valid_email_address',
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
  '#description' => t('Enter a preference for seating. Example: "Middle Section- 3rd Row".<br /><strong>Please Note:</strong> The comma character (,) <em>cannot</em> be used in this field.'),
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
  '#prefix' => '<div style="width: 100%; float:left">',
  '#field_prefix' => t('$'),
  '#suffix' => '</div>',
  '#description' => t('Enter an amount of donation in USD in the format ##.## (Note this donation <strong>is</strong> tax deductible).'),
 );

 $form['reqexpl'] = array(
  '#markup' => t('Elements with an astrick (') . '<span class="form-required" title="This field is required.">' . t('*') . '</span>' . t(') are required.'),
  '#prefix' => '<p>',
  '#suffix' => '</p>',
 );

 $form['order'] = array(
  '#type' => 'submit',
  '#value' => t('Order Tickets'),
 );

 return $form;
}
function tasks_tickets_order_form_validate($form, &$form_state) {
  //Check for zip code and donation amount to be numeric
  if( !is_numeric($form_state['values']['hzip']) ) {
    form_set_error('hzip', t('Field <em>Home Zip Code</em> is a numeric only field.'));
  }
  if( !is_numeric($form_state['values']['amount']) ) {
    form_set_error('hzip', t('Field <em>Donation Amount</em> is a numeric only field.'));
  }
  if( preg_match("/,/i", $form_state['values']['seat']) ) {
    form_set_error('seat', t('The comma character (,) is not allowed in the <em>Seat</em> field.'));
  }
}
function tasks_tickets_order_form_submit($form, &$form_state) {
$txn = db_transaction();

try {
  // Build Orderid
  $orderid = rand(1,4) . rand (3,8) . rand(7,12) . rand(11, 16) . '-' . $form_state['values']['lname'];

  // Build total price
  $total = 0;

  $dontation_int = substr($form_state['values']['amount'], 1);
  if($dontation_int > 0) {
    $total += $dontation_int;
  }

  $ticket_total = $form_state['values']['qty'] * 35;

  if($ticket_total > 0) {
   $total += $ticket_total;
  }

  if ($form_state['values']['show'] == 'Thursday Night') {
    $nightnum = 1;
  }
  if ($form_state['values']['show'] == 'Friday Night') {
    $nightnum = 2;
  }
  if ($form_state['values']['show'] == 'Saturday Matinee') {
    $nightnum = 3;
  }
  if ($form_state['values']['show'] == 'Saturday Night') {
    $nightnum = 4;
  }
  if ($form_state['values']['show'] == 'Sunday Matinee') {
    $nightnum = 5;
  }

  $query = db_insert('tickets_orders');
  $query
  ->fields(array(
   'orderid'     => $orderid,
   'seasonid'    => variable_get('tasks_tickets_season_for_sale_id'),
   'fname'       => $form_state['values']['fname'],
   'lname'       => $form_state['values']['lname'],
   'email'       => $form_state['values']['email'],
   'address'     => $form_state['values']['haddress'],
   'city'        => $form_state['values']['hcity'],
   'state'       => $form_state['values']['hstate'],
   'zip'         => $form_state['values']['hzip'],
   'night'       => $form_state['values']['show'],
   'nightnum'    => $nightnum,
   'preference'  => $form_state['values']['seat'],
   'qty'         => $form_state['values']['qty'],
   'donation'    => $form_state['values']['amount'],
   'total'       => $total,
   'paid'        => 0,
  ))
  ->execute();

  $query = 'SELECT gid FROM {email_groups}  LIMIT 1';
  $results = db_query($query);
  foreach ($results as $row) {
   $rowid = $row->gid;
  }

  $query = db_insert('email_members');
  $query
    ->fields(array(
       'eid' => NULL,
       'gid' => $rowid,
       'name' => $form_state['values']['fname'] . ' ' . $form_state['values']['lname'],
       'email' => $form_state['values']['email'],
    ))
    ->execute();

  drupal_goto('box_office/purchase/download', array('query' => array('orderid' => $orderid)));
 }
  catch (execption $e) {
    $txn->rollback;
    watchdog_exception('php', $e);
    drupal_set_message(t('Your order could not be completed. Please contact webmaster@rultc.org with the follwing message @date', array('@date' => date(U))), 'error');
  }
}

function tasks_download_page() {
 $query = db_select('tickets_orders', 't');
 $query
   ->fields('t', array('unique_id', 'orderid', 'seasonid', 'fname', 'lname', 'email', 'address', 'city', 'state', 'night', 'qty', 'donation', 'total', 'timestamp'))
   ->condition('orderid', $_GET['orderid']);

 $results = array();
 $results = $query->execute()->fetchAssoc($results);

 $build = array();

 $build['intro'] = array(
  '#type' => 'markup',
  '#markup' => t('Congratulations. You have resereved @qty season tickets under the name @lname. To complete this purchase you need to download the following form and mail it, along with a check or
                 money order for $@total, to:'
                 , array('@qty' => $results['qty'], '@lname' => $results['lname'], '@total' => $results['total'])). '<br><blockquote>' .
               t('The Livingston Theatre Company<br />84 Joyce Kilmer Avenue Room 121<br />Piscataway, NJ 08854') . '</blockquote></p>'
 );

 $query = db_select('tickets_orders', 't');
 $query
   ->fields('t', array('unique_id', 'orderid', 'fname', 'lname', 'email', 'address', 'city', 'preference', 'state', 'night', 'qty', 'donation', 'total'))
   ->condition('orderid', $_GET['orderid']);

 $res = $query->execute();

 foreach ($res as $r) {
  $rows1[] = array($r->orderid, $r->fname, $r->lname, $r->email, $r->address, $r->city, $r->state, $r->donation, $r->total);
  $rows2[] = array($r->orderid, $r->night, $r->preference, $r->qty, $r->total);
 }

 $build['temp1'] = array(
   '#prefix' => '<p>Please print this table and keep for you reference. Be sure to note your order number on your check or money order to assist in processing your order</p><p>',
   '#suffix' => '</p>',
   '#theme' => 'table',
   '#rows' => $rows1,
   '#header' => array('Order ID', 'First Name', 'Last Name', 'Email', 'Address', 'City', 'State', 'Donation', 'Order Total'),
 );

 $build['temp2'] = array(
   '#theme' => 'table',
   '#rows' => $rows2,
   '#header' => array('Order ID', 'Night', 'Location Preference', 'Quantity', 'Order Total'),
 );

 /* TODO: Views_pdf fields placement
 $build['link'] = array(
  '#type' => 'markup',
  '#markup' => '<p>' . t('Please make sure you payment notes your Order ID: @orderid. Please keep the following form for your records:', array('@orderid' => $results['orderid'])) . '<br />'.
               l(t('Invoice-@orderid.pdf', array('@orderid' => $results['orderid'])), 'box_office/purchase/download/pdf', array('query' => array('orderid' => $results['orderid'])))  . '</p>',
 );

 $build['note'] = array(
  '#type' => 'markup',
  '#markup' => '<p>' . t('Note that a PDF reader (such as ') . l('Adobe Acrobat', 'http://get.adobe.com/reader/') . t(') is required to view the invoice.') . '</p>',
 );
 */

 $build['contact'] = array(
   '#type' => 'markup',
   '#markup' => '<p>' . t('At this point we <em>are not</em> able to accept credit card as a payment process. If you cannot provide payment in the form of cash of
                          money order please email use at ') . l('boxoffice@rultc.org', 'mailto:boxoffice@rultc.org') . t(' so we can help resolve your situations. If you have any questions
                          about this process please send an email to ') . l('boxoffice@rultc.org', 'mailto:boxoffice@rultc.org') . '</p>',
 );

 $build['text'] = t('Order @id', array('@id' => $_GET['orderid']));

	$build['note'] = array(
		'#type' => 'markup',
		'#markup' => t('<p> Once we recieve you payment your status in our database will be changed to paid, and
                you will promplty be notified by the email address which you provided. If you have made a donation a letter for your tax records stating the amount of your dontaion (if one is made) will be mailed to you. If you have any
                questions feel free to email <a href="mailto:boxoffice@rultc.org.">boxoffice@rultc.org</a> and your questions will be promptly answered. Thank you for your patronage.</p>'),
	);

 return $build;
}