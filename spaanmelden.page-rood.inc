<?php
// Aanmeldformulier voor ROOD-leden

function spaanmelden_page_rood() {
	return drupal_get_form('spaanmelden_form_rood');
}

function spaanmelden_form_rood($form, &$form_state) {
	$steps = spaanmelden_form_rood_form_steps();

	if(!isset($form_state['storage']['step'])) {
		$_steps = array_keys($steps);
		$form_state['storage']['step'] = $_steps[0];
	}

	$step = $form_state['storage']['step'];

	$intro = variable_get('spaanmelden_text_rood');
	if($intro) {
       $form['introduction'] = array(
        '#prefix' => "<div class=\"introduction\">",
        '#markup' => check_markup($intro['value'],$intro['format']),
        '#suffix' => "</div>",
      );
    }

	$form['ajax_container'] = [
		'#prefix'   => '<div id="spaanmelden_form">',
		'#suffix'   => '</div>',
		'#attached' => [
			'library' => [
				['system', 'effects'],
			],
			'js'      => [
				drupal_get_path('module', 'spaanmelden') . '/spaanmelden.js',
			]
		],
	];

	$form['ajax_container']['progress'] = [
		'#theme'       => 'spaanmelden_progress',
		'#steps'       => $steps,
		'#active_step' => $step,
		'#weight'      => 0,
	];

	$form['ajax_container']['actions'] = [
		'#type'   => 'actions',
		'#weight' => 1000,
	];

	if(spaanmelden_form_rood_previous_step($step)) {
		$form['ajax_container']['actions']['previous'] = [
			'#type'                    => 'submit',
			'#value'                   => 'Vorige',
			'#submit'                  => ['spaanmelden_form_rood_submit_previous'],
			'#limit_validation_errors' => [],
			'#attributes'              => ['class' => ['btn', 'small', 'hollow', 'prev-btn']],
			'#ajax'                    => [
				'wrapper'  => 'spaanmelden_form',
				'callback' => 'spaanmelden_form_rood_form_ajax_callback',
			],
		];
	}

	if(spaanmelden_form_rood_next_step($step)) {
		$form['ajax_container']['actions']['next'] = [
			'#type'       => 'submit',
			'#value'      => 'Volgende',
			'#submit'     => ['spaanmelden_form_rood_submit_next'],
			'#validate'   => ['spaanmelden_form_rood_validate_next'],
			'#attributes' => ['class' => ['btn', 'small', 'next-btn']],
			'#ajax'       => [
				'wrapper'  => 'spaanmelden_form',
				'callback' => 'spaanmelden_form_rood_form_ajax_callback',
			],
		];
	} else {
		$form['ajax_container']['actions']['submit'] = [
			'#type'       => 'submit',
			'#value'      => 'Akkoord',
			'#attributes' => ['class' => ['btn', 'submit-btn']],
		];
	}

	$callback = $steps[$step]['callback'];
	$form['ajax_container'] = $callback($form['ajax_container'], $form_state);

	return $form;
}

function spaanmelden_form_rood_submit($form, &$form_state) {

	// Correcties / aanpassingen in veldinvoer vppr uniformiteit

	// Membership type
	$memberships = _spaanmelden_form_rood_available_memberships($form_state);
	$membership_type = ($memberships['SP'] ? 'lid_sprood' : 'lid_rood');

	// Postcode
	if(drupal_array_get_nested_value($form_state['storage']['data'], ['address', 'country']) == 'NL') {
		$postcode = drupal_array_get_nested_value($form_state['storage']['data'], ['address', 'zipcode']);
		$postcode = preg_replace('/(?<=[a-z])(?=\d)|(?<=\d)(?=[a-z])/i', ' ', $postcode);
	} else {
		$postcode = drupal_array_get_nested_value($form_state['storage']['data'], ['address', 'country']) . '-' . drupal_array_get_nested_value($form_state['storage']['data'], ['address', 'zipcode']);
	}
	$postcode = strtoupper($postcode);

	// Plaats
	$city = drupal_array_get_nested_value($form_state['storage']['data'], ['address', 'city']);
	$city = strtoupper($city);

	// Bijdrage
	$contribution = drupal_array_get_nested_value($form_state['storage']['data'], ['payment', 'contribution']);
	if($contribution == 'other') {
		$contribution = floatval(drupal_array_get_nested_value($form_state['storage']['data'], ['payment', 'contribution_other']));
	} else {
		$contribution = intval($contribution) / 100.0;
	}

	// Welkomstcadeau
	$welcome_present_sp = drupal_array_get_nested_value($form_state['storage']['data'], ['welcome_present', 'normal']);
	if($membership_type == 'lid_sprood') {
		$welcome_present_rood = drupal_array_get_nested_value($form_state['storage']['data'], ['welcome_present', 'rood']);
	} else {
		$welcome_present_rood = null;
	}

	// IBAN formatting
	$bank_account = drupal_array_get_nested_value($form_state['storage']['data'], ['payment', 'bank_account']);
	$bank_account = strtoupper(preg_replace('/\s+/', '', $bank_account));

	// Name and email (used in several places)
	$first_name = drupal_array_get_nested_value($form_state['storage']['data'], ['personal_information', 'firstname']);
	$last_name = drupal_array_get_nested_value($form_state['storage']['data'], ['personal_information', 'lastname']);
	$initials = drupal_array_get_nested_value($form_state['storage']['data'], ['personal_information', 'initials']);
	$email = drupal_array_get_nested_value($form_state['storage']['data'], ['contact', 'email']);

	// Overige variabelen
	$record = [
		'membership_type'      => $membership_type,
		'hash'                 => _spaanmelden_get_hash($membership_type, $last_name, $email),
		'initials'             => $initials,
		'firstname'            => $first_name,
		'middlename'           => drupal_array_get_nested_value($form_state['storage']['data'], ['personal_information', 'middlename']),
		'lastname'             => $last_name,
		'gender'               => drupal_array_get_nested_value($form_state['storage']['data'], ['personal_information', 'gender']),
		'birthdate'            => drupal_array_get_nested_value($form_state['storage']['data'], ['personal_information', 'birthdate']),
		'email'                => $email,
		'phone'                => drupal_array_get_nested_value($form_state['storage']['data'], ['contact', 'phone']),
		'phone_mobile'         => drupal_array_get_nested_value($form_state['storage']['data'], ['contact', 'phone_mobile']),
		'country'              => drupal_array_get_nested_value($form_state['storage']['data'], ['address', 'country']),
		'zipcode'              => $postcode,
		'housenumber'          => drupal_array_get_nested_value($form_state['storage']['data'], ['address', 'housenumber']),
		'housenumber_suffix'   => drupal_array_get_nested_value($form_state['storage']['data'], ['address', 'housenumber_suffix']),
		'street'               => drupal_array_get_nested_value($form_state['storage']['data'], ['address', 'street']),
		'city'                 => $city,
		'contribution'         => $contribution,
		'bank_account'         => $bank_account,
		'payment_method'       => drupal_array_get_nested_value($form_state['storage']['data'], ['payment', 'payment_method']),
		'welcome_present_sp'   => $welcome_present_sp,
		'welcome_present_rood' => $welcome_present_rood,
	];

	// Submit record
	$ret = _spaanmelden_insert_entry($record);

	if($ret) {

		$mail = drupal_mail(
			'spaanmelden', 'verificatie', $email, LANGUAGE_NONE,
			[
				'form_state'      => $form_state,
				'hash'            => $record['hash'],
				'membership_type' => 'lid_rood',
				'format'          => SWIFTMAILER_FORMAT_HTML,
			]
		);

		if(!empty($mail['send'])) {

			$name = $first_name ? $first_name : $initials . ' ' . $last_name;
			$form_state['redirect'] = [
				'aanmelden/bedankt',
				['query' => ['name' => base64_encode($name), 'membership_type' => $membership_type, 'token' => drupal_get_token($name)]]
			];
			return true;
		}
	}

	$form_state['redirect'] = ['aanmelden/fout'];
}

function spaanmelden_form_rood_form_ajax_callback($form, &$form_state) {

	$result = $form['ajax_container'];
	$commands = ajax_prepare_response($result);

	// Scroll up on errors
	$noscrollTriggers = ['birthdate', 'country', 'zipcode', 'housenumber', 'payment_method'];
	$noScroll = false;
	foreach($noscrollTriggers as $nsTrigger) {
		if(in_array($nsTrigger, $form_state['triggering_element']['#parents']))
			$noScroll = true;
	}
	if(!$noScroll) {
		array_unshift($commands, ['command' => 'scrollToWrapper']);
	}

	// Focus on element after postcode callback
	$postcodeTriggers = ['country', 'zipcode', 'housenumber'];
	foreach($postcodeTriggers as $trigger) {
		if(in_array($trigger, $form_state['triggering_element']['#parents'])) {
			array_unshift($commands, ['command' => 'focusElement', 'element' => 'form-item-address-' . $trigger]);
			break;
		}
	}

	$output = [
		'#type'     => 'ajax',
		'#commands' => $commands,
	];

	return $output;
}

function spaanmelden_form_rood_submit_next($form, &$form_state) {
	spaanmelden_form_rood_submit_progress($form, $form_state);

	$step = $form_state['storage']['step'];
	$next_step = spaanmelden_form_rood_next_step($step);

	if($next_step) {
		$form_state['storage']['step'] = $next_step;
	}
	$form_state['rebuild'] = true;
}

function spaanmelden_form_rood_validate_next($form, &$form_state) {
	spaanmelden_form_rood_validate_progress($form, $form_state);
}

function spaanmelden_form_rood_validate_progress($form, &$form_state) {
	$steps = spaanmelden_form_rood_form_steps();
	$step = $form_state['storage']['step'];
	$callback = $steps[$step]['callback'] . '_validate';

	if(function_exists($callback)) {
		$callback($form, $form_state);
	}
}

function spaanmelden_form_rood_submit_progress($form, &$form_state) {
	$steps = spaanmelden_form_rood_form_steps();
	$step = $form_state['storage']['step'];
	$callback = $steps[$step]['callback'] . '_submit';
	if(function_exists($callback)) {
		$callback($form, $form_state);
	}
}

function spaanmelden_form_rood_submit_previous($form, &$form_state) {
	$step = $form_state['storage']['step'];
	$previous_step = spaanmelden_form_rood_previous_step($step);

	if($previous_step) {
		$form_state['storage']['step'] = $previous_step;
	}
	$form_state['rebuild'] = true;
}

function spaanmelden_form_rood_form_steps() {
	$steps = [];

	$steps['personal_information'] = [
		'title'    => 'Naam en geboortedatum',
		'callback' => 'spaanmelden_form_rood_form_personal_information',
	];

	$steps['donation'] = [
		'title'    => 'Kies uw bijdrage',
		'callback' => 'spaanmelden_form_rood_form_donation',
	];

	$steps['gift'] = [
		'title'    => 'Kies uw cadeau',
		'callback' => 'spaanmelden_form_rood_form_gift',
	];

	$steps['review'] = [
		'title'    => 'Controleer uw gegevens',
		'callback' => 'spaanmelden_form_rood_form_review',
	];

	return $steps;
}

function spaanmelden_form_rood_next_step($current_step) {
	return spaanmelden_form_rood_other_step($current_step, 1);
}

function spaanmelden_form_rood_previous_step($current_step) {
	return spaanmelden_form_rood_other_step($current_step, - 1);
}

function spaanmelden_form_rood_other_step($current_step, $offset = 0) {
	$steps = spaanmelden_form_rood_form_steps();
	$_steps = array_keys($steps);

	foreach($_steps as $step_id => $step) {
		if($step == $current_step) {
			if(!empty($_steps[$step_id + $offset])) {
				return $_steps[$step_id + $offset];
			}

			return false;
		}
	}

	return false;
}

function spaanmelden_form_rood_form_personal_information($form, &$form_state) {

	$text = null;
	$showm = null;
	if(!empty($form_state['input']['personal_information']['birthdate'])) {
		$showm = _spaanmelden_form_rood_available_memberships($form_state);

		if(!is_array($showm)) {
			$text = '';
		} elseif($showm['SP'] && $showm['ROOD']) {
			$text = 'Je bent jonger dan 28 jaar en wordt zowel lid van de SP als lid van ROOD. ROOD-lid zijn kost je niets extra\'s en je krijgt bovendien een ROOD-shirt als welkomstcadeau!';
		} elseif(!$showm['SP'] && !$showm['ROOD']) {
			$text = 'Je bent jonger dan 14 jaar en kunt daarom op dit moment nog niet lid worden van de SP of ROOD.';
		} elseif($showm['ROOD']) {
			$text = 'Je bent jonger dan 16 jaar en kunt nu alleen ROOD-lid worden.';
		} elseif($showm['SP']) {
			$text = 'Je bent ouder dan 28 jaar en kunt daarom geen ROOD-lid worden. Ga naar het <a href="' . url('aanmelden/lid') . '">SP-aanmeldformulier</a> om lid te worden van de SP.';
		}
	}

	$form['personal_information'] = [
		'#type'  => 'fieldset',
		'#title' => 'Persoonsgegevens',
		'#tree'  => true,
	    '#access' => (!$showm || $showm['ROOD']),
	];

	$form['personal_information']['initials'] = [
		'#title'         => 'Voorletters',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, ['personal_information', 'initials'], ''),
		'#required'      => true,
	];

	$form['personal_information']['firstname'] = [
		'#title'         => 'Voornaam',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, ['personal_information', 'firstname'], ''),
		'#required'      => true,
	];

	$form['personal_information']['middlename'] = [
		'#title'         => 'Tussenvoegsel',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, ['personal_information', 'middlename'], ''),
		'#required'      => false,
	];

	$form['personal_information']['lastname'] = [
		'#title'         => 'Achternaam',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, ['personal_information', 'lastname'], ''),
		'#required'      => true,
	];

	$form['personal_information']['gender'] = [
		'#title'         => 'Geslacht',
		'#type'          => 'radios',
		'#options'       => ['m' => 'Man', 'v' => 'Vrouw', 'a' => 'Anders'],
		'#default_value' => _spaanmelden_form_default_value($form_state, ['personal_information', 'gender'], null),
		'#required'      => true,
	];

	$form['personal_information']['birthdate'] = [
		'#title'           => 'Geboortedatum',
		'#type'            => 'date_select',
		'#date_format'     => 'd-m-Y',
		'#date_year_range' => '-28:-13',
		'#default_value'   => _spaanmelden_form_default_value($form_state, ['personal_information', 'birthdate'], ''),
		'#required'        => true,
		'#ajax'            => [
			'wrapper'  => 'spaanmelden_form',
			'callback' => 'spaanmelden_form_rood_form_ajax_callback',
			'progress' => 'none',
		],
	];

	$form['rood'] = [
		'#type'        => 'fieldset',
		'#title'       => 'Jouw lidmaatschap',
		'#attributes'  => ['class' => ['doemee-lid-worden-rood']],
		'#access'      => ($text ? true : false),
		'#description' => $text,
	];

	$form['contact'] = [
		'#type'  => 'fieldset',
		'#title' => 'Contactgegevens',
		'#tree'  => true,
	    '#access' => (!$showm || $showm['ROOD']),
	];

	$form['contact']['email'] = [
		'#title'         => 'E-mailadres',
		'#type'          => 'emailfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, ['contact', 'email'], []),
		'#required'      => true,
	];

	$form['contact']['email_confirm'] = array(
		'#title'         => 'Bevestig je e-mailadres',
		'#type'          => 'emailfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, array('contact', 'email_confirm'), array()),
		'#required'      => true,
	);

	$form['contact']['phone'] = [
		'#title'         => 'Telefoonnummer',
		'#type'          => 'telfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, ['contact', 'phone'], []),
		'#required'      => false,
	];

	$form['contact']['phone_mobile'] = [
		'#title'         => 'Mobiele telefoonnummer',
		'#type'          => 'telfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, ['contact', 'phone_mobile'], []),
		'#required'      => false,
	];

	$form['address'] = [
		'#type'  => 'fieldset',
		'#title' => 'Adresgegevens',
		'#tree'  => true,
		'#access' => (!$showm || $showm['ROOD']),
	];

	// Necessary for country_get_list().
	require_once DRUPAL_ROOT . '/includes/locale.inc';
	$countries = country_get_list();

	$country = _spaanmelden_form_default_value($form_state, ['address', 'country'], 'NL');
	if(!empty($form_state['input']['address']['country']))
		$country = $form_state['input']['address']['country'];

	$form['address']['country'] = [
		'#title'         => 'Land',
		'#type'          => 'select',
		'#options'       => $countries,
		'#default_value' => $country,
		'#required'      => true,
		'#attributes'    => ['style' => 'float: none !important;'],
		'#ajax'          => [
			'wrapper'  => 'spaanmelden_form',
			'callback' => 'spaanmelden_form_rood_form_ajax_callback',
			'progress' => 'none',
		],
	];

	$zipcode = _spaanmelden_form_default_value($form_state, ['address', 'zipcode'], '');
	if(!empty($form_state['input']['address']['zipcode']))
		$zipcode = $form_state['input']['address']['zipcode'];
	$form['address']['zipcode'] = [
		'#title'         => 'Postcode',
		'#type'          => 'textfield',
		'#default_value' => $zipcode,
		'#required'      => true,
		'#attributes'    => ['style' => 'float: none !important;'],
		'#ajax'          => [
			'wrapper'  => 'spaanmelden_form',
			'callback' => 'spaanmelden_form_rood_form_ajax_callback',
			'progress' => 'none',
		],
	];

	$housenumber = _spaanmelden_form_default_value($form_state, ['address', 'housenumber'], '');
	if(!empty($form_state['input']['address']['housenumber']))
		$housenumber = $form_state['input']['address']['housenumber'];
	$form['address']['housenumber'] = [
		'#title'         => 'Huisnummer',
		'#type'          => 'textfield',
		'#default_value' => $housenumber,
		'#required'      => true,
		'#attributes'    => ['style' => 'float: none !important;'],
		'#ajax'          => [
			'wrapper'  => 'spaanmelden_form',
			'callback' => 'spaanmelden_form_rood_form_ajax_callback',
			'progress' => 'none',
		],
	];

	$form['address']['housenumber_suffix'] = [
		'#title'         => 'Huisnummer toevoeging',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, ['address', 'housenumber_suffix'], ''),
		'#required'      => false,
	];

	$form['address']['description'] = [
		'#prefix' => '<div id="spaanmelden_lookup">',
		'#markup' => 'Uw straat- en plaatsnaam worden automatisch ingevuld.',
		'#suffix' => '</div>',
	];

	// Ajax postcode lookup
	if($country == 'NL') {
		$disabled = true;

		if($zipcode && $housenumber) {

			$lookup = _spaanmelden_postcode_lookup($zipcode, $housenumber);
			if($lookup) {
				$city = $lookup->woonplaats;
				$street = $lookup->adres;
				$form['address']['description']['#markup'] = 'Uw straat- en plaatsnaam zijn automatisch ingevuld.';
			} else {
				$city = '';
				$street = '';
				$form['address']['description']['#markup'] = 'Uw straat- en plaatsnaam konden niet worden gevonden. Controleer uw invoer.';
			}
		} else {
			$city = '';
			$street = '';
			$form['address']['description']['#markup'] = 'Uw straat- en plaatsnaam worden automatisch ingevuld.';
		}
	} else {

		$city = _spaanmelden_form_default_value($form_state, ['address', 'city'], '');
		$street = _spaanmelden_form_default_value($form_state, ['address', 'street'], '');
		$disabled = false;
		$form['address']['description']['#markup'] = 'Voor adressen buiten Nederland moet u zelf de straat- en plaatsnaam invullen.';
	}

	$form['address']['street'] = [
		'#title'         => 'Straat',
		'#type'          => 'textfield',
		'#default_value' => $street,
		'#required'      => true,
		'#disabled'      => $disabled,
	];

	$form['address']['city'] = [
		'#title'         => 'Plaats',
		'#type'          => 'textfield',
		'#default_value' => $city,
		'#required'      => true,
		'#disabled'      => $disabled,
	];

	return $form;
}

function spaanmelden_form_rood_form_personal_information_tables(&$info, &$form_state) {
	$form = spaanmelden_form_rood_form_personal_information([], $form_state);

	foreach(element_children($form) as $fieldset_key) {
		$fieldset = $form[$fieldset_key];
		if($fieldset['#type'] != 'fieldset' || (isset($fieldset['#access']) && empty($fieldset['#access']))) {
			continue;
		}

		$info[check_plain($fieldset['#title'])] = _spaanmelden_fieldset_table($fieldset);
	}
}

function spaanmelden_form_rood_form_personal_information_submit($form, &$form_state) {
	if(!isset($form_state['storage']['data'])) {
		$form_state['storage']['data'] = [];
	}

	foreach(['personal_information', 'contact', 'address', 'rood'] as $key) {
		$form_state['storage']['data'][$key] = $form_state['values'][$key];
	}
}

function spaanmelden_form_rood_form_personal_information_validate($form, &$form_state) {
	$housenumber = drupal_array_get_nested_value($form_state['values'], ['address', 'housenumber']);
	if(!empty($housenumber)) {
		$housenumber = preg_replace('/[^0-9]/', '', $housenumber);
		$form_state['input']['address']['housenumber'] = $housenumber;
		$form_state['values']['address']['housenumber'] = $housenumber;

		if(empty($housenumber) || $housenumber < 0) {
			form_set_error('address][housenumber', 'Er is geen geldig huisnummer ingevoerd.');
		}
	}

	$zipcode = drupal_array_get_nested_value($form_state['values'], ['address', 'zipcode']);
	$country = drupal_array_get_nested_value($form_state['values'], ['address', 'country']);
	if(!preg_match('/^[1-9][0-9]{3}\s?([a-zA-Z]{2})?$/i', $zipcode) && $country == 'NL') {
		form_set_error('address][zipcode', 'Er is geen geldige postcode ingevoerd.');
	}

	$phone = drupal_array_get_nested_value($form_state['values'], ['contact', 'phone']);
	if(!empty($phone)) {
		$phone = preg_replace('/[^0-9]/', '', $phone);
		if(empty($phone) || strlen($phone) < 10) {
			form_set_error('contact][phone', 'Er is geen geldig telefoonnummer ingevoerd.');
		}
	}

	$phone_mobile = drupal_array_get_nested_value($form_state['values'], ['contact', 'phone_mobile']);
	if(!empty($phone_mobile)) {
		$phone_mobile = preg_replace('/[^0-9]/', '', $phone_mobile);
		if(empty($phone_mobile)) {
			form_set_error('contact][phone_mobile', 'Er is geen geldig mobiele telefoonnummer ingevoerd.');
		}
	}

	$email = drupal_array_get_nested_value($form_state['values'], array('contact', 'email'));
	$email_confirm = drupal_array_get_nested_value($form_state['values'], array('contact', 'email_confirm'));
	if($email != $email_confirm) {
		form_set_error('contact][email_confirm', 'Vul twee keer hetzelfde emailadres in.');
	}
}

function spaanmelden_form_rood_form_donation($form, &$form_state) {

	$form['payment'] = [
		'#type'  => 'fieldset',
		'#title' => 'Betalingsgegevens',
		'#tree'  => true,
	];


	$showm = _spaanmelden_form_rood_available_memberships($form_state);

	$form['payment']['contribution'] = [
		'#title'         => 'Bijdrage per ' . ((!$showm['SP'] && $showm['ROOD']) ? 'jaar' : 'kwartaal'),
		'#type'          => 'radios',
		'#options'       => [
			'500'   => '&euro; 5,00 (minimum)', '750' => '&euro; 7,50',
			'1000'  => '&euro; 10,00', '1250' => '&euro; 12,50', '1500' => '&euro; 15,00',
			'other' => 'Anders, namelijk:'
		],
		'#default_value' => _spaanmelden_form_default_value($form_state, ['payment', 'contribution'], 500),
		'#required'      => true,
	];

	$form['payment']['contribution_other'] = [
		'#title'         => '€',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, ['payment', 'contribution_other'], ''),
		'#required'      => false,
		'#states'        => [
			'enabled' => [
				':input[name="payment[contribution]"]' => ['value' => 'other'],
			]
		],
	];

	$options = ['incasso' => 'Automatische incasso', 'acceptgiro' => 'Acceptgiro (1 euro per kwartaal administratiekosten)'];
	if(!$showm['SP'] && $showm['ROOD'])
		unset($options['acceptgiro']);

	$form['payment']['payment_method'] = [
		'#title'         => 'Ik betaal met',
		'#type'          => 'radios',
		'#options'       => $options,
		'#default_value' => _spaanmelden_form_default_value($form_state, ['payment', 'payment_method'], null),
		'#required'      => true,
		'#ajax'          => [
			'wrapper'  => 'spaanmelden_form',
			'callback' => 'spaanmelden_form_rood_form_ajax_callback',
			'progress' => 'none',
		],
	];

	$form['payment']['bank_account'] = [
		'#title'         => 'Rekeningnummer (IBAN)',
		'#type'          => 'textfield',
		'#default_value' => _spaanmelden_form_default_value($form_state, ['payment', 'bank_account'], ''),
		'#required'      => true,
		'#access'        => _spaanmelden_form_is_incasso($form_state),
		'#description'   => '<p>Vind uw IBAN nummer op de site <a href="http://www.openiban.nl/" target="_blank">OpenIBAN</a></p>',
	];

	return $form;
}

function spaanmelden_form_rood_form_donation_tables(&$info, &$form_state) {
	$form = spaanmelden_form_rood_form_donation([], $form_state);

	foreach(element_children($form) as $fieldset_key) {
		$fieldset = $form[$fieldset_key];
		if($fieldset['#type'] != 'fieldset' || (isset($fieldset['#access']) && empty($fieldset['#access']))) {
			continue;
		}

		$rows = [];

		$contribution = _spaanmelden_form_default_value($form_state, ['payment', 'contribution'], 500);
		if($contribution == 'other') {
			$contribution = floatval(_spaanmelden_form_default_value($form_state, ['payment', 'contribution_other'], ''));
		} else {
			$contribution = intval($contribution) / 100.0;
		}

		$rows['Bijdrage'] = '&euro; ' . number_format($contribution, 2, ',', '.');
		$rows = array_merge($rows, _spaanmelden_fieldset_table($fieldset, ['bank_account', 'payment_method']));
		$info[check_plain($fieldset['#title'])] = $rows;
	}
}

function spaanmelden_form_rood_form_donation_review(&$info, &$form_state) {
	$form = spaanmelden_form_rood_form_donation([], $form_state);

	$contribution = _spaanmelden_form_default_value($form_state, ['payment', 'contribution'], 500);
	if($contribution == 'other') {
		$contribution = floatval(_spaanmelden_form_default_value($form_state, ['payment', 'contribution_other'], ''));
	} else {
		$contribution = intval($contribution) / 100.0;
	}

	$memberships = _spaanmelden_form_rood_available_memberships($form_state);
	$description = _spaanmelden_membership_description($memberships['SP'] ? 'lid_sprood' : 'lid_rood');
	$period = $memberships['SP'] ? 'per kwartaal' : 'per jaar';
	$info['payment'] = [
		'#markup' => '<p class="payment-info">Ik word ' . $description . '. Ik betaal ' . $period . ' &euro; ' . number_format($contribution, 2, ',', '.') . ' aan contributie.</p><p>Indien je per incasso betaalt, ontvang je twee weken voor de eerste afschrijving via internetbankieren bericht wanneer de contributie wordt afgeschreven.</p>',
		'#weight' => 100,
	];
}

function spaanmelden_form_rood_form_donation_validate($form, &$form_state) {
	$contribution = drupal_array_get_nested_value($form_state['values'], ['payment', 'contribution']);
	if($contribution == 'other') {
		$contribution_other = drupal_array_get_nested_value($form_state['values'], ['payment', 'contribution_other']);
		if(empty($contribution_other)) {
			form_set_error('payment][contribution_other', 'Vul alsjeblieft een bedrag in dat je per kwartaal wil bijdragen.');
		} elseif(floatval($contribution_other) < 5) {
			form_set_error('payment][contribution_other', 'De minimale kwartaalbijdrage is &euro; 5.');
		}
	}

	$payment_method = drupal_array_get_nested_value($form_state['values'], ['payment', 'payment_method']);
	$bank_account = drupal_array_get_nested_value($form_state['values'], ['payment', 'bank_account']);
	if($payment_method == 'incasso' && !_spaanmelden_check_iban($bank_account)) {
		form_set_error('payment][bank_account', 'Er is geen geldig rekeningnummer ingevoerd. U dient een IBAN nummer in te voeren.');
	}
}

function spaanmelden_form_rood_form_donation_submit($form, &$form_state) {
	if(!isset($form_state['storage']['data'])) {
		$form_state['storage']['data'] = [];
	}

	foreach(['payment'] as $key) {
		$form_state['storage']['data'][$key] = $form_state['values'][$key];
	}
}

function spaanmelden_form_rood_form_gift($form, &$form_state) {
	$form['welcome_present'] = [
		'#type'  => 'fieldset',
		'#title' => 'Welkomstcadeau',
		'#tree'  => true,
	];

	$form['welcome_present']['intro'] = [
		'#title'  => 'Welkomstcadeau',
		'#type'   => 'item',
		'#markup' => '<p>Kies uit één van de onderstaande welkomstcadeaus.</p>',
	];

	$showm = _spaanmelden_form_rood_available_memberships($form_state);

	$form['welcome_present']['normal'] = [
		'#title'         => 'Welkomstcadeau SP',
		'#type'          => 'select',
		'#options'       => _spaanmelden_get_gifts('lid_sp'),
		'#default_value' => _spaanmelden_form_default_value($form_state, ['welcome_present', 'normal'], ''),
		'#required'      => true,
	    '#access'       => $showm['SP'],
	];

	$form['welcome_present']['rood'] = [
		'#title'         => 'Welkomstcadeau ROOD',
		'#type'          => 'select',
		'#options'       => _spaanmelden_get_gifts('lid_rood'),
		'#default_value' => _spaanmelden_form_default_value($form_state, ['welcome_present', 'rood'], ''),
		'#required'      => true,
		'#access'        => $showm['ROOD'],
	];

	return $form;
}

function spaanmelden_form_rood_form_gift_tables(&$info, &$form_state) {
	$form = spaanmelden_form_rood_form_gift([], $form_state);

	foreach(element_children($form) as $fieldset_key) {
		$fieldset = $form[$fieldset_key];
		if($fieldset['#type'] != 'fieldset' || (isset($fieldset['#access']) && empty($fieldset['#access']))) {
			continue;
		}

		$info[check_plain($fieldset['#title'])] = _spaanmelden_fieldset_table($fieldset);
	}
}

function spaanmelden_form_rood_form_gift_submit($form, &$form_state) {
	if(!isset($form_state['storage']['data'])) {
		$form_state['storage']['data'] = [];
	}

	foreach(['welcome_present'] as $key) {
		$form_state['storage']['data'][$key] = $form_state['values'][$key];
	}
}

function spaanmelden_form_rood_form_review($form, &$form_state) {
	$steps = spaanmelden_form_rood_form_steps();
	$form['info'] = [];

	foreach($steps as $key => $info) {
		$callback = $info['callback'] . '_review';

		if(function_exists($callback)) {
			$callback($form['info'], $form_state);
		}
	}

	return $form;
}

function spaanmelden_form_rood_form_review_review(&$info, &$form_state) {
	$info['tables'] = _spaanmelden_format_tables('lid_rood', $form_state);
	$info['#weight'] = 10;
}

function _spaanmelden_form_rood_available_memberships(&$form_state) {
	$birthdate = _spaanmelden_form_default_value($form_state, ['personal_information', 'birthdate'], '');
	$birthdate = $birthdate ? (array_combine(['year', 'month', 'day'], explode('-', $birthdate))) : [];
	if(!empty($form_state['input']['personal_information']['birthdate'])) {
		$birthdate = array_map('intval', $form_state['input']['personal_information']['birthdate']);
	}
	if(!$birthdate || !$birthdate['year'] || !$birthdate['month'] || !$birthdate['day'])
		return null;

	$birthdate = new \DateTime($birthdate['year'] . '-' . $birthdate['month'] . '-' . $birthdate['day']);
	$now = new \DateTime;

	$interval = $birthdate->diff($now);
	if($interval->y >= 16 && $interval->y <= 27) {
		return ['SP' => 1, 'ROOD' => 1];
	} elseif($interval->y < 14) {
		return ['SP' => 0, 'ROOD' => 0];
	} elseif($interval->y <= 16) {
		return ['SP' => 0, 'ROOD' => 1];
	} elseif($interval->y > 27) {
		return ['SP' => 1, 'ROOD' => 0];
	}
}
