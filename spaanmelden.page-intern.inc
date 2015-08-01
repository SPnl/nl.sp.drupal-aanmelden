<?php
// Intern snel aanmeldformulier voor de ledenadministratie zelf.

function spaanmelden_page_intern() {
	return drupal_get_form('spaanmelden_form_intern');
}

function spaanmelden_form_intern($form, &$form_state) {
	$steps = spaanmelden_form_intern_form_steps();

	if (!isset($form_state['storage']['step'])) {
		$_steps = array_keys($steps);
		$form_state['storage']['step'] = $_steps[0];
	}

	$step = $form_state['storage']['step'];

	$intro = variable_get('spaanmelden_text_intern');
	if ($intro) {
		$form['introtext'] = array(
			'#markup' => $intro['value'],
		);
	}

	$form['ajax_container'] = array(
		'#prefix'   => '<div id="spaanmelden_form">',
		'#suffix'   => '</div>',
		'#attached' => array(
			'library' => array(
				array('system', 'effects'),
			),
			'js'      => array(
				drupal_get_path('module', 'spaanmelden') . '/spaanmelden.js',
			)
		),
	);

	$form['ajax_container']['progress'] = array(
		'#theme'       => 'spaanmelden_progress',
		'#steps'       => $steps,
		'#active_step' => $step,
		'#weight'      => 0,
	);

	$form['ajax_container']['actions'] = array(
		'#type'   => 'actions',
		'#weight' => 1000,
	);

	if (spaanmelden_form_intern_previous_step($step)) {
		$form['ajax_container']['actions']['previous'] = array(
			'#type'                    => 'submit',
			'#value'                   => 'Vorige',
			'#submit'                  => array('spaanmelden_form_intern_submit_previous'),
			'#limit_validation_errors' => array(),
			'#attributes'              => array('class' => array('btn', 'small', 'hollow', 'prev-btn')),
			'#ajax'                    => array(
				'wrapper'  => 'spaanmelden_form',
				'callback' => 'spaanmelden_form_intern_form_ajax_callback',
			),
		);
	}

	if (spaanmelden_form_intern_next_step($step)) {
		$form['ajax_container']['actions']['next'] = array(
			'#type'       => 'submit',
			'#value'      => 'Volgende',
			'#submit'     => array('spaanmelden_form_intern_submit_next'),
			'#validate'   => array('spaanmelden_form_intern_validate_next'),
			'#attributes' => array('class' => array('btn', 'small', 'next-btn')),
			'#ajax'       => array(
				'wrapper'  => 'spaanmelden_form',
				'callback' => 'spaanmelden_form_intern_form_ajax_callback',
			),
		);
	} else {
		$form['ajax_container']['actions']['submit'] = array(
			'#type'       => 'submit',
			'#value'      => 'Akkoord',
			'#attributes' => array('class' => array('btn', 'submit-btn')),
		);
	}

	$callback = $steps[$step]['callback'];
	$form['ajax_container'] = $callback($form['ajax_container'], $form_state);

	return $form;
}

function spaanmelden_form_intern_submit($form, &$form_state) {

	// Laatste pagina opslaan (want we hebben geen review)

	if (!isset($form_state['storage']['data'])) {
		$form_state['storage']['data'] = array();
	}

	foreach (array('member', 'payment') as $key) {
		$form_state['storage']['data'][ $key ] = $form_state['values'][ $key ];
	}

	// Correcties / aanpassingen in veldinvoer voor uniformiteit

	// Membership type
	$membership_type = drupal_array_get_nested_value($form_state['storage']['data'], array('member', 'membership_type'));

	// Postcode
	if (drupal_array_get_nested_value($form_state['storage']['data'], array('address', 'country')) == 'NL') {
		$postcode = drupal_array_get_nested_value($form_state['storage']['data'], array('address', 'zipcode'));
		$postcode = preg_replace('/(?<=[a-z])(?=\d)|(?<=\d)(?=[a-z])/i', ' ', $postcode);
	} else {
		$postcode = drupal_array_get_nested_value($form_state['storage']['data'], array('address', 'country')) . '-' . drupal_array_get_nested_value($form_state['storage']['data'], array('address', 'zipcode'));
	}
	$postcode = strtoupper($postcode);

	// Plaats
	$city = drupal_array_get_nested_value($form_state['storage']['data'], array('address', 'city'));
	$city = strtoupper($city);

	// Bijdrage
	$contribution = drupal_array_get_nested_value($form_state['storage']['data'], array('payment', 'contribution'));
	if ($contribution == 'other') {
		$contribution = floatval(drupal_array_get_nested_value($form_state['storage']['data'], array('payment', 'contribution_other')));
	} else {
		$contribution = intval($contribution) / 100.0;
	}

	// Welkomstcadeau
	$welcome_present_sp = drupal_array_get_nested_value($form_state['storage']['data'], array('member','welcome_normal'));
	if ($membership_type == 'lid_sprood') {
		$welcome_present_rood = drupal_array_get_nested_value($form_state['storage']['data'], array('member','welcome_rood'));
	} else {
		$welcome_present_rood = null;
	}

	// Nieuw: bron
	$source = drupal_array_get_nested_value($form_state['storage']['data'], array('member', 'source'));

	// IBAN formatting
	$bank_account = drupal_array_get_nested_value($form_state['storage']['data'], array('payment', 'bank_account'));
	$bank_account = strtoupper(preg_replace('/\s+/', '', $bank_account));

	// Name and email (used in several places)
	$first_name = drupal_array_get_nested_value($form_state['storage']['data'], array('personal', 'firstname'));
	$last_name = drupal_array_get_nested_value($form_state['storage']['data'], array('personal', 'lastname'));
	$initials = drupal_array_get_nested_value($form_state['storage']['data'], array('personal', 'initials'));

	$email = drupal_array_get_nested_value($form_state['storage']['data'], array('contact', 'email'));
	if(!$email)
		$email = null;

	// Overige variabelen
	$record = array(
		'membership_type'      => $membership_type,
		'hash'                 => _spaanmelden_get_hash($membership_type, $last_name, $email),
		'initials'             => $initials,
		'firstname'            => $first_name,
		'middlename'           => drupal_array_get_nested_value($form_state['storage']['data'], array('personal','middlename')),
		'lastname'             => $last_name,
		'gender'               => drupal_array_get_nested_value($form_state['storage']['data'], array('personal','gender')),
		'birthdate'            => drupal_array_get_nested_value($form_state['storage']['data'], array('personal','birthdate')),
		'email'                => $email,
		'phone'                => drupal_array_get_nested_value($form_state['storage']['data'], array('contact','phone')),
		'phone_mobile'         => drupal_array_get_nested_value($form_state['storage']['data'], array('contact','phone_mobile')),
		'country'              => drupal_array_get_nested_value($form_state['storage']['data'], array('address','country')),
		'zipcode'              => $postcode,
		'housenumber'          => drupal_array_get_nested_value($form_state['storage']['data'], array('address','housenumber')),
		'housenumber_suffix'   => drupal_array_get_nested_value($form_state['storage']['data'], array('address','housenumber_suffix')),
		'street'               => drupal_array_get_nested_value($form_state['storage']['data'], array('address','street')),
		'city'                 => $city,
		'contribution'         => $contribution,
		'bank_account'         => $bank_account,
		'payment_method'       => drupal_array_get_nested_value($form_state['storage']['data'], array('payment','payment_method')),
		'welcome_present_sp'   => $welcome_present_sp,
		'welcome_present_rood' => $welcome_present_rood,
		'source'               => $source,
		'verified'             => time(),
		'flags'                => 'INTERN',
	);

	// Submit record
	$ret = _spaanmelden_insert_entry($record);

	if ($ret) {

		drupal_set_message('Het contact en lidmaatschap zijn opgeslagen. Je kunt nu een nieuw lid invoeren.');
		$form_state['redirect'] = array('aanmelden/intern');
		return true;
	}

	$form_state['redirect'] = array('aanmelden/fout');
}

function spaanmelden_form_intern_form_ajax_callback($form, &$form_state) {

	$result = $form['ajax_container'];
	$commands = ajax_prepare_response($result);

	// Scroll up on errors
	$noscrollTriggers = array('birthdate', 'country', 'zipcode', 'housenumber', 'payment_method', 'email', 'membership_type');
	$noScroll = false;
	foreach ($noscrollTriggers as $nsTrigger) {
		if (in_array($nsTrigger, $form_state['triggering_element']['#parents']))
			$noScroll = true;
	}
	if (!$noScroll) {
		array_unshift($commands, array('command' => 'scrollToWrapper'));
	}

	// Focus on element after postcode callback
	$postcodeTriggers = array('country', 'zipcode', 'housenumber');
	foreach ($postcodeTriggers as $trigger) {
		if (in_array($trigger, $form_state['triggering_element']['#parents'])) {
			array_unshift($commands, array('command' => 'focusElement', 'element' => 'form-item-address-' . $trigger));
			break;
		}
	}

	// Focus on element after email callback
	if (in_array('email', $form_state['triggering_element']['#parents'])) {
		array_unshift($commands, array('command' => 'focusElement', 'element' => 'form-item-contact-email'));
	}

	$output = array(
		'#type'     => 'ajax',
		'#commands' => $commands,
	);

	return $output;
}

function spaanmelden_form_intern_submit_next($form, &$form_state) {
	spaanmelden_form_intern_submit_progress($form, $form_state);

	$step = $form_state['storage']['step'];
	$next_step = spaanmelden_form_intern_next_step($step);

	if ($next_step) {
		$form_state['storage']['step'] = $next_step;
	}
	$form_state['rebuild'] = true;
}

function spaanmelden_form_intern_validate_next($form, &$form_state) {
	spaanmelden_form_intern_validate_progress($form, $form_state);
}

function spaanmelden_form_intern_validate_progress($form, &$form_state) {
	$steps = spaanmelden_form_intern_form_steps();
	$step = $form_state['storage']['step'];
	$callback = $steps[ $step ]['callback'] . '_validate';

	if (function_exists($callback)) {
		$callback($form, $form_state);
	}
}

function spaanmelden_form_intern_submit_progress($form, &$form_state) {
	$steps = spaanmelden_form_intern_form_steps();
	$step = $form_state['storage']['step'];
	$callback = $steps[ $step ]['callback'] . '_submit';
	if (function_exists($callback)) {
		$callback($form, $form_state);
	}
}

function spaanmelden_form_intern_submit_previous($form, &$form_state) {
	$step = $form_state['storage']['step'];
	$previous_step = spaanmelden_form_intern_previous_step($step);

	if ($previous_step) {
		$form_state['storage']['step'] = $previous_step;
	}
	$form_state['rebuild'] = true;
}

function spaanmelden_form_intern_form_steps() {
	$steps = array();

	$steps['contact'] = array(
		'title'    => 'Contactgegevens',
		'callback' => 'spaanmelden_form_intern_form_contact',
	);

	$steps['membership'] = array(
		'title'    => 'Lidmaatschapsgegevens',
		'callback' => 'spaanmelden_form_intern_form_membership',
	);

	return $steps;
}

function spaanmelden_form_intern_next_step($current_step) {
	return spaanmelden_form_intern_other_step($current_step, 1);
}

function spaanmelden_form_intern_previous_step($current_step) {
	return spaanmelden_form_intern_other_step($current_step, - 1);
}

function spaanmelden_form_intern_other_step($current_step, $offset = 0) {
	$steps = spaanmelden_form_intern_form_steps();
	$_steps = array_keys($steps);

	foreach ($_steps as $step_id => $step) {
		if ($step == $current_step) {
			if (!empty($_steps[ $step_id + $offset ])) {
				return $_steps[ $step_id + $offset ];
			}

			return false;
		}
	}

	return false;
}

function spaanmelden_form_intern_form_contact($form, &$form_state) {
	$form['personal'] = array(
		'#type'  => 'fieldset',
		'#title' => 'Persoonsgegevens',
		'#tree'  => true,
	);

	$form['personal']['initials'] = array(
		'#title'         => 'Voorletters',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, array('personal', 'initials'), ''),
		'#required'      => false,
	);

	$form['personal']['firstname'] = array(
		'#title'         => 'Voornaam',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, array('personal', 'firstname'), ''),
		'#required'      => false,
	);

	$form['personal']['middlename'] = array(
		'#title'         => 'Tussenvoegsel',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, array('personal', 'middlename'), ''),
		'#required'      => false,
	);

	$form['personal']['lastname'] = array(
		'#title'         => 'Achternaam',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, array('personal', 'lastname'), ''),
		'#required'      => true,
	);

	$form['personal']['gender'] = array(
		'#title'         => 'Geslacht',
		'#type'          => 'radios',
		'#options'       => array('m' => 'Man', 'v' => 'Vrouw', 'a' => 'Anders'),
		'#default_value' => _spaanmelden_form_default_value($form_state, array('personal', 'gender'), null),
		'#required'      => false,
	);

	$form['personal']['birthdate'] = array(
		'#title'           => 'Geboortedatum',
		'#type'            => 'date_select',
		'#date_format'     => 'd-m-Y',
		'#date_year_range' => '-100:-15',
		'#default_value'   => _spaanmelden_form_default_value($form_state, array('personal', 'birthdate'), ''),
		'#required'        => false,
	);

	$form['contact'] = array(
		'#type'  => 'fieldset',
		'#title' => 'Contactgegevens',
		'#tree'  => true,
	);

	$form['contact']['doubles_msg'] = array(
		'#prefix' => '<div id="spaanmelden_contact_doublesmsg">',
		'#markup' => '',
		'#suffix' => '</div>',
	);

	$emailAddress = _spaanmelden_form_default_value($form_state, array('contact', 'email'), '');
	if (!empty($form_state['input']['contact']['email']))
		$emailAddress = $form_state['input']['contact']['email'];

	$form['contact']['email'] = array(
		'#title'         => 'E-mailadres',
		'#type'          => 'textfield', // emailfield werkt niet met callback
		'#default_value' => $emailAddress,
		'#required'      => false,
		'#ajax'          => array(
			'wrapper'  => 'spaanmelden_form',
			'callback' => 'spaanmelden_form_intern_form_ajax_callback',
		),
	);

	// Checken op dubbelen op basis van emailadres
	if ($emailAddress) {
		$emailDoublesCheck = _spaanmelden_doubles_check_email($emailAddress);
		if($emailDoublesCheck) {
			$form['contact']['doubles_msg']['#markup'] = '<em>Let op, ' . (count($emailDoublesCheck) == 1 ? 'er bestaat al een contact' : 'er bestaan al ' . count($emailDoublesCheck) . ' contacten') . ' met dit emailadres: ' . implode(', ', $emailDoublesCheck) . '.</em>';
		} else {
			$form['contact']['doubles_msg']['#markup'] = '';
		}
	}

	$form['contact']['phone'] = array(
		'#title'         => 'Telefoonnummer',
		'#type'          => 'telfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, array('contact', 'phone'), array()),
		'#required'      => false,
	);

	$form['contact']['phone_mobile'] = array(
		'#title'         => 'Mobiele telefoonnummer',
		'#type'          => 'telfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, array('contact', 'phone_mobile'), array()),
		'#required'      => false,
	);

	$form['address'] = array(
		'#type'  => 'fieldset',
		'#title' => 'Adresgegevens',
		'#tree'  => true,
	);

	// Necessary for country_get_list().
	require_once DRUPAL_ROOT . '/includes/locale.inc';
	$countries = country_get_list();

	$country = _spaanmelden_form_default_value($form_state, array('address', 'country'), 'NL');
	if (!empty($form_state['input']['address']['country']))
		$country = $form_state['input']['address']['country'];

	$form['address']['country'] = array(
		'#title'         => 'Land',
		'#type'          => 'select',
		'#options'       => $countries,
		'#default_value' => $country,
		'#required'      => true,
		'#attributes'    => array('style' => 'float: none !important;'),
		'#ajax'          => array(
			'wrapper'  => 'spaanmelden_form',
			'callback' => 'spaanmelden_form_intern_form_ajax_callback',
		),
	);

	$form['address']['doubles_msg'] = array(
		'#prefix' => '<div id="spaanmelden_address_doublesmsg">',
		'#markup' => '',
		'#suffix' => '</div>',
	);

	$zipcode = _spaanmelden_form_default_value($form_state, array('address', 'zipcode'), '');
	if (!empty($form_state['input']['address']['zipcode']))
		$zipcode = $form_state['input']['address']['zipcode'];
	$form['address']['zipcode'] = array(
		'#title'         => 'Postcode',
		'#type'          => 'textfield',
		'#default_value' => $zipcode,
		'#required'      => true,
		'#attributes'    => array('style' => 'float: none !important;'),
		'#ajax'          => array(
			'wrapper'  => 'spaanmelden_form',
			'callback' => 'spaanmelden_form_intern_form_ajax_callback',
		),
	);

	$housenumber = _spaanmelden_form_default_value($form_state, array('address', 'housenumber'), '');
	if (!empty($form_state['input']['address']['housenumber']))
		$housenumber = $form_state['input']['address']['housenumber'];
	$form['address']['housenumber'] = array(
		'#title'         => 'Huisnummer',
		'#type'          => 'textfield',
		'#default_value' => $housenumber,
		'#required'      => true,
		'#attributes'    => array('style' => 'float: none !important;'),
		'#ajax'          => array(
			'wrapper'  => 'spaanmelden_form',
			'callback' => 'spaanmelden_form_intern_form_ajax_callback',
		),
	);

	$form['address']['housenumber_suffix'] = array(
		'#title'         => 'Huisnummer toevoeging',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, array('address', 'housenumber_suffix'), ''),
		'#required'      => false,
	);

	$form['address']['description'] = array(
		'#prefix' => '<div id="spaanmelden_lookup">',
		'#markup' => 'De straat- en plaatsnaam worden automatisch ingevuld.',
		'#suffix' => '</div>',
	);

	// Ajax postcode lookup
	if ($country == 'NL') {
		$disabled = true;

		if ($zipcode && $housenumber) {

			// Postcode lookup
			$lookup = _spaanmelden_postcode_lookup($zipcode, $housenumber);
			if ($lookup) {
				$city = $lookup->woonplaats;
				$street = $lookup->adres;
				$form['address']['description']['#markup'] = 'De straat- en plaatsnaam zijn automatisch ingevuld.';
			} else {
				$city = '';
				$street = '';
				$form['address']['description']['#markup'] = 'De straat- en plaatsnaam konden niet worden gevonden. Controleer de invoer.';
			}

			// Doubles check
			$doublesCheck = _spaanmelden_doubles_check($zipcode, $housenumber);
			if($doublesCheck) {
				$form['address']['doubles_msg']['#markup'] = '<em>Let op, ' . (count($doublesCheck) == 1 ? 'er bestaat al een contact' : 'er bestaan al ' . count($doublesCheck) . ' contacten') . ' op dit adres: ' . implode(', ', $doublesCheck) . '.</em>';
			} else {
				$form['address']['doubles_msg']['#markup'] = '';
			}

		} else {
			$city = '';
			$street = '';
			$form['address']['description']['#markup'] = 'De straat- en plaatsnaam worden automatisch ingevuld.';
		}
	} else {

		$city = _spaanmelden_form_default_value($form_state, array('address', 'city'), '');
		$street = _spaanmelden_form_default_value($form_state, array('address', 'street'), '');
		$disabled = false;
		$form['address']['description']['#markup'] = 'Voor adressen buiten Nederland moet u zelf de straat- en plaatsnaam invullen.';
	}

	$form['address']['street'] = array(
		'#title'         => 'Straat',
		'#type'          => 'textfield',
		'#default_value' => $street,
		'#required'      => true,
		'#disabled'      => $disabled,
	);

	$form['address']['city'] = array(
		'#title'         => 'Plaats',
		'#type'          => 'textfield',
		'#default_value' => $city,
		'#required'      => true,
		'#disabled'      => $disabled,
	);

	return $form;
}

function spaanmelden_form_intern_form_contact_tables(&$info, &$form_state) {
	$form = spaanmelden_form_intern_form_contact(array(), $form_state);

	foreach (element_children($form) as $fieldset_key) {
		$fieldset = $form[ $fieldset_key ];
		if ($fieldset['#type'] != 'fieldset' || (isset($fieldset['#access']) && empty($fieldset['#access']))) {
			continue;
		}

		$info[ check_plain($fieldset['#title']) ] = _spaanmelden_fieldset_table($fieldset);
	}
}

function spaanmelden_form_intern_form_contact_submit($form, &$form_state) {
	if (!isset($form_state['storage']['data'])) {
		$form_state['storage']['data'] = array();
	}

	foreach (array('personal', 'contact', 'address') as $key) {
		$form_state['storage']['data'][ $key ] = $form_state['values'][ $key ];
	}
}

function spaanmelden_form_intern_form_contact_validate($form, &$form_state) {
	$housenumber = drupal_array_get_nested_value($form_state['values'], array('address', 'housenumber'));
	if (!empty($housenumber)) {
		$housenumber = preg_replace('/[^0-9]/', '', $housenumber);
		$form_state['input']['address']['housenumber'] = $housenumber;
		$form_state['values']['address']['housenumber'] = $housenumber;

		if (empty($housenumber) || $housenumber < 0) {
			form_set_error('address][housenumber', 'Er is geen geldig huisnummer ingevoerd.');
		}
	}

	$zipcode = drupal_array_get_nested_value($form_state['values'], array('address', 'zipcode'));
	$country = drupal_array_get_nested_value($form_state['values'], array('address', 'country'));
	if (!preg_match('/^[1-9][0-9]{3}\s?([a-zA-Z]{2})?$/i', $zipcode) && $country == 'NL') {
		form_set_error('address][zipcode', 'Er is geen geldige postcode ingevoerd.');
	}

	$phone = drupal_array_get_nested_value($form_state['values'], array('contact', 'phone'));
	if (!empty($phone)) {
		$phone = preg_replace('/[^0-9]/', '', $phone);
		if (empty($phone) || strlen($phone) < 10) {
			form_set_error('contact][phone', 'Er is geen geldig telefoonnummer ingevoerd.');
		}
	}

	$phone_mobile = drupal_array_get_nested_value($form_state['values'], array('contact', 'phone_mobile'));
	if (!empty($phone_mobile)) {
		$phone_mobile = preg_replace('/[^0-9]/', '', $phone_mobile);
		if (empty($phone_mobile)) {
			form_set_error('contact][phone_mobile', 'Er is geen geldig mobiel telefoonnummer ingevoerd.');
		}
	}

	$email = drupal_array_get_nested_value($form_state['values'], array('contact', 'email'));
	if (!empty($email)) {
		if (preg_match('^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$', $email)) {
			form_set_error('contact][email', 'Er is geen geldig e-mailadres ingevoerd.');
		}
	}
}

function spaanmelden_form_intern_form_membership($form, &$form_state) {

	$form['member'] = array(
		'#type'  => 'fieldset',
		'#title' => 'Lidmaatschap',
		'#tree'  => true,
	);

	$membershipType = _spaanmelden_form_default_value($form_state, array('member', 'membership_type'), 'lid_sp');
	if (!empty($form_state['input']['member']['membership_type']))
		$membershipType = $form_state['input']['member']['membership_type'];

	$form['member']['membership_type'] = array(
		'#type'          => 'select',
		'#title'         => 'Lidmaatschapstype',
		'#required'      => true,
		'#default_value' => $membershipType,
		'#options'       => array(
			'lid_sp'     => 'Lid SP',
			'lid_sprood' => 'Lid SP en ROOD',
			'tribune'    => 'Abonnee Tribune',
			'spanning'   => 'Abonnee Spanning',
		),
		'#ajax'          => array(
			'wrapper'  => 'spaanmelden_form',
			'callback' => 'spaanmelden_form_intern_form_ajax_callback',
			'progress' => 'none',
		),
	);

	$form['member']['welcome_normal'] = array(
		'#title'         => 'Welkomstcadeau SP',
		'#type'          => 'select',
		'#options'       => _spaanmelden_get_gifts('lid_sp', false),
		'#default_value' => _spaanmelden_form_default_value($form_state, array('member', 'welcome_normal'), ''),
		'#required'      => true,
		'#access'        => in_array($membershipType, array('lid_sp', 'lid_sprood')),
	);

	$form['member']['welcome_rood'] = array(
		'#title'         => 'Welkomstcadeau ROOD',
		'#type'          => 'select',
		'#options'       => _spaanmelden_get_gifts('lid_rood', false),
		'#default_value' => _spaanmelden_form_default_value($form_state, array('member', 'welcome_rood'), ''),
		'#required'      => true,
		'#access'        => ($membershipType == 'lid_sprood'),
	);

	$form['member']['source'] = array(
		'#title'         => 'Bron',
		'#type'          => 'select',
		'#options'       => _spaanmelden_get_source_options(),
		'#default_value' => _spaanmelden_form_default_value($form_state, array('member', 'source'), ''),
		'#required'      => false,
	);

	$form['payment'] = array(
		'#type'  => 'fieldset',
		'#title' => 'Betalingsgegevens',
		'#tree'  => true,
	);

	$form['payment']['contribution'] = array(
		'#title'         => 'Bijdrage per kwartaal',
		'#type'          => 'radios',
		'#options'       => array(
			'500'   => '&euro; 5,00 (minimum)',
			'750'   => '&euro; 7,50',
			'1000'  => '&euro; 10,00',
			'1250'  => '&euro; 12,50',
			'1500'  => '&euro; 15,00',
			'other' => 'Anders, namelijk:'
		),
		'#default_value' => _spaanmelden_form_default_value($form_state, array('payment', 'contribution'), 500),
		'#required'      => true,
		'#access'        => in_array($membershipType, array('lid_sp', 'lid_sprood')),
	);

	$form['payment']['contribution_other'] = array(
		'#title'         => '€',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, array('payment', 'contribution_other'), ''),
		'#required'      => false,
		'#states'        => array(
			'enabled' => array(
				':input[name="payment[contribution]"]' => array('value' => 'other'),
			)
		),
		'#access'        => in_array($membershipType, array('lid_sp', 'lid_sprood')),
	);

	$paymentMethod = drupal_array_get_nested_value($form_state, array('values', 'payment', 'payment_method'));
	$form['payment']['payment_method'] = array(
		'#title'         => 'Ik betaal met',
		'#type'          => 'radios',
		'#options'       => array(
			'incasso'    => 'Automatische incasso',
			'acceptgiro' => 'Acceptgiro (&euro; 24 per jaar)'
		),
		'#default_value' => $paymentMethod,
		'#required'      => true,
		'#ajax'          => array(
			'wrapper'  => 'spaanmelden_form',
			'callback' => 'spaanmelden_form_intern_form_ajax_callback',
			'progress' => 'none',
		),
	);

	$form['payment']['bank_account'] = array(
		'#title'         => 'Rekeningnummer (IBAN)',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, array('payment', 'bank_account'), ''),
		'#required'      => true,
		'#access'        => ($paymentMethod == 'incasso'),
		'#description'   => '<p><small>Vind het IBAN nummer op de site <a href="https://www.ibanbicservice.nl/SingleRequest.aspx" target="_blank">IBAN BIC Service</a></small></p>',
	);

	return $form;
}

function spaanmelden_form_intern_form_membership_tables(&$info, &$form_state) {
	$form = spaanmelden_form_intern_form_membership(array(), $form_state);

	foreach (element_children($form) as $fieldset_key) {
		$fieldset = $form[ $fieldset_key ];
		if ($fieldset['#type'] != 'fieldset' || (isset($fieldset['#access']) && empty($fieldset['#access']))) {
			continue;
		}

		$rows = array();

		$contribution = _spaanmelden_form_default_value($form_state, array('payment', 'contribution'), 500);
		if ($contribution == 'other') {
			$contribution = floatval(_spaanmelden_form_default_value($form_state, array('payment','contribution_other'), ''));
		} else {
			$contribution = intval($contribution) / 100.0;
		}

		$rows['Bijdrage per kwartaal'] = '&euro; ' . number_format($contribution, 2, ',', '.');
		$rows = array_merge($rows, _spaanmelden_fieldset_table($fieldset, array('bank_account', 'payment_method')));
		$info[ check_plain($fieldset['#title']) ] = $rows;
	}
}

function spaanmelden_form_intern_form_membership_validate($form, &$form_state) {
	$contribution = drupal_array_get_nested_value($form_state['values'], array('payment', 'contribution'));
	if ($contribution == 'other') {
		$contribution_other = drupal_array_get_nested_value($form_state['values'], array('payment','contribution_other'));
		if (empty($contribution_other)) {
			form_set_error('payment][contribution_other', 'Vul alsjeblieft een bedrag in dat je per kwartaal wil bijdragen.');
		} elseif (floatval($contribution_other) < 5) {
			form_set_error('payment][contribution_other', 'De minimale kwartaalbijdrage is &euro; 5.');
		}
	}

	$payment_method = drupal_array_get_nested_value($form_state['values'], array('payment', 'payment_method'));
	$bank_account = drupal_array_get_nested_value($form_state['values'], array('payment', 'bank_account'));
	if ($payment_method == 'incasso' && !_spaanmelden_check_iban($bank_account)) {
		form_set_error('payment][bank_account', 'Er is geen geldig rekeningnummer ingevoerd. U dient een IBAN nummer in te voeren.');
	}
}
