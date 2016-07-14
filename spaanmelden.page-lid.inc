<?php
// Aanmeldformulier voor SP- / ROOD-leden
// Grotendeels overgenomen van https://github.com/SPWebteam/sp-website/tree/v.1.0.0/modules/features/sections/doemee

function spaanmelden_page_lid() {
    return drupal_get_form('spaanmelden_form_lid');
}

function spaanmelden_form_lid($form, &$form_state) {
    $steps = spaanmelden_form_lid_form_steps();

    if(!isset($form_state['storage']['step'])) {
        $_steps = array_keys($steps);
        $form_state['storage']['step'] = $_steps[0];
    }

    $step = $form_state['storage']['step'];

    $intro = variable_get('spaanmelden_text_lid');
    if($intro) {
       $form['introduction'] = array(
        '#prefix' => "<div class=\"introduction\">",
        '#markup' => check_markup($intro['value'],$intro['format']),
        '#suffix' => "</div>",
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

    if(spaanmelden_form_lid_previous_step($step)) {
        $form['ajax_container']['actions']['previous'] = array(
            '#type'                    => 'submit',
            '#value'                   => 'Vorige',
            '#submit'                  => array('spaanmelden_form_lid_submit_previous'),
            '#limit_validation_errors' => array(),
            '#attributes'              => array('class' => array('btn', 'small', 'hollow', 'prev-btn')),
            '#ajax'                    => array(
                'wrapper'  => 'spaanmelden_form',
                'callback' => 'spaanmelden_form_lid_form_ajax_callback',
            ),
        );
    }

    if(spaanmelden_form_lid_next_step($step)) {
        $form['ajax_container']['actions']['next'] = array(
            '#type'       => 'submit',
            '#value'      => 'Volgende',
            '#submit'     => array('spaanmelden_form_lid_submit_next'),
            '#validate'   => array('spaanmelden_form_lid_validate_next'),
            '#attributes' => array('class' => array('btn', 'small', 'next-btn')),
            '#ajax'       => array(
                'wrapper'  => 'spaanmelden_form',
                'callback' => 'spaanmelden_form_lid_form_ajax_callback',
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

function spaanmelden_form_lid_submit($form, &$form_state) {

    // Correcties / aanpassingen in veldinvoer vppr uniformiteit

    // Membership type
    $rood = drupal_array_get_nested_value($form_state['storage']['data'], array('rood', 'rood'));
    if($rood) {
        $membership_type = 'lid_sprood';
    } else {
        $membership_type = 'lid_sp';
    }

    // Postcode
    if(drupal_array_get_nested_value($form_state['storage']['data'], array('address', 'country')) == 'NL') {
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
    if($contribution == 'other') {
        $contribution = floatval(drupal_array_get_nested_value($form_state['storage']['data'], array('payment', 'contribution_other')));
    } else {
        $contribution = intval($contribution) / 100.0;
    }

    // Welkomstcadeau
    $welcome_present_sp = drupal_array_get_nested_value($form_state['storage']['data'], array('welcome_present', 'normal'));
    if($membership_type == 'lid_sprood') {
        $welcome_present_rood = drupal_array_get_nested_value($form_state['storage']['data'], array('welcome_present', 'rood'));
    } else {
        $welcome_present_rood = null;
    }

    // IBAN formatting
    $bank_account = drupal_array_get_nested_value($form_state['storage']['data'], array('payment', 'bank_account'));
    $bank_account = strtoupper(preg_replace('/\s+/', '', $bank_account));

    // Name and email (used in several places)
    $first_name = drupal_array_get_nested_value($form_state['storage']['data'], array('personal_information', 'firstname'));
    $last_name = drupal_array_get_nested_value($form_state['storage']['data'], array('personal_information', 'lastname'));
    $initials = drupal_array_get_nested_value($form_state['storage']['data'], array('personal_information', 'initials'));
    $email = drupal_array_get_nested_value($form_state['storage']['data'], array('contact', 'email'));

    // Overige variabelen
    $record = array(
        'membership_type'      => $membership_type,
        'hash'                 => _spaanmelden_get_hash($membership_type, $last_name, $email),
        'initials'             => $initials,
        'firstname'            => $first_name,
        'middlename'           => drupal_array_get_nested_value($form_state['storage']['data'], array('personal_information', 'middlename')),
        'lastname'             => $last_name,
        'gender'               => drupal_array_get_nested_value($form_state['storage']['data'], array('personal_information', 'gender')),
        'birthdate'            => drupal_array_get_nested_value($form_state['storage']['data'], array('personal_information', 'birthdate')),
        'email'                => $email,
        'phone'                => drupal_array_get_nested_value($form_state['storage']['data'], array('contact', 'phone')),
        'phone_mobile'         => drupal_array_get_nested_value($form_state['storage']['data'], array('contact', 'phone_mobile')),
        'country'              => drupal_array_get_nested_value($form_state['storage']['data'], array('address', 'country')),
        'zipcode'              => $postcode,
        'housenumber'          => drupal_array_get_nested_value($form_state['storage']['data'], array('address', 'housenumber')),
        'housenumber_suffix'   => drupal_array_get_nested_value($form_state['storage']['data'], array('address', 'housenumber_suffix')),
        'street'               => drupal_array_get_nested_value($form_state['storage']['data'], array('address', 'street')),
        'city'                 => $city,
        'contribution'         => $contribution,
        'bank_account'         => $bank_account,
        'payment_method'       => drupal_array_get_nested_value($form_state['storage']['data'], array('payment', 'payment_method')),
        'welcome_present_sp'   => $welcome_present_sp,
        'welcome_present_rood' => $welcome_present_rood,
    );

    // Submit record
    $ret = _spaanmelden_insert_entry($record);

    if($ret) {

        $mail = drupal_mail(
            'spaanmelden', 'verificatie', $email, LANGUAGE_NONE,
            array(
                'form_state'      => $form_state,
                'hash'            => $record['hash'],
                'membership_type' => $membership_type,
                'format'          => SWIFTMAILER_FORMAT_HTML,
            )
        );

        if(!empty($mail['send'])) {

            $name = $first_name ? $first_name : $initials . ' ' . $last_name;
            $form_state['redirect'] = array('aanmelden/bedankt',
                array('query' => array('name' => base64_encode($name), 'membership_type' => $membership_type, 'token' => drupal_get_token($name)))
            );
            return true;
        }
    }

    $form_state['redirect'] = array('aanmelden/fout');
}

function spaanmelden_form_lid_form_ajax_callback($form, &$form_state) {

    $result = $form['ajax_container'];
    $commands = ajax_prepare_response($result);

    // Scroll up on errors
    $noscrollTriggers = array('birthdate', 'country', 'zipcode', 'housenumber', 'payment_method');
    $noScroll = false;
    foreach($noscrollTriggers as $nsTrigger) {
        if(in_array($nsTrigger, $form_state['triggering_element']['#parents']))
            $noScroll = true;
    }
    if(!$noScroll) {
        array_unshift($commands, array('command' => 'scrollToWrapper'));
    }

    // Focus on element after postcode callback
    $postcodeTriggers = array('country', 'zipcode', 'housenumber');
    foreach($postcodeTriggers as $trigger) {
        if(in_array($trigger, $form_state['triggering_element']['#parents'])) {
            array_unshift($commands, array('command' => 'focusElement', 'element' => 'form-item-address-' . $trigger));
            break;
        }
    }

    $output = array(
        '#type'     => 'ajax',
        '#commands' => $commands,
    );

    return $output;
}

function spaanmelden_form_lid_submit_next($form, &$form_state) {
    spaanmelden_form_lid_submit_progress($form, $form_state);

    $step = $form_state['storage']['step'];
    $next_step = spaanmelden_form_lid_next_step($step);

    if($next_step) {
        $form_state['storage']['step'] = $next_step;
    }
    $form_state['rebuild'] = true;
}

function spaanmelden_form_lid_validate_next($form, &$form_state) {
    spaanmelden_form_lid_validate_progress($form, $form_state);
}

function spaanmelden_form_lid_validate_progress($form, &$form_state) {
    $steps = spaanmelden_form_lid_form_steps();
    $step = $form_state['storage']['step'];
    $callback = $steps[$step]['callback'] . '_validate';

    if(function_exists($callback)) {
        $callback($form, $form_state);
    }
}

function spaanmelden_form_lid_submit_progress($form, &$form_state) {
    $steps = spaanmelden_form_lid_form_steps();
    $step = $form_state['storage']['step'];
    $callback = $steps[$step]['callback'] . '_submit';
    if(function_exists($callback)) {
        $callback($form, $form_state);
    }
}

function spaanmelden_form_lid_submit_previous($form, &$form_state) {
    $step = $form_state['storage']['step'];
    $previous_step = spaanmelden_form_lid_previous_step($step);

    if($previous_step) {
        $form_state['storage']['step'] = $previous_step;
    }
    $form_state['rebuild'] = true;
}

function spaanmelden_form_lid_form_steps() {
    $steps = array();

    $steps['personal_information'] = array(
        'title'    => 'Naam en geboortedatum',
        'callback' => 'spaanmelden_form_lid_form_personal_information',
    );

    $steps['donation'] = array(
        'title'    => 'Kies uw bijdrage',
        'callback' => 'spaanmelden_form_lid_form_donation',
    );

    $steps['gift'] = array(
        'title'    => 'Kies uw cadeau',
        'callback' => 'spaanmelden_form_lid_form_gift',
    );

    $steps['review'] = array(
        'title'    => 'Controleer uw gegevens',
        'callback' => 'spaanmelden_form_lid_form_review',
    );

    return $steps;
}

function spaanmelden_form_lid_next_step($current_step) {
    return spaanmelden_form_lid_other_step($current_step, 1);
}

function spaanmelden_form_lid_previous_step($current_step) {
    return spaanmelden_form_lid_other_step($current_step, -1);
}

function spaanmelden_form_lid_other_step($current_step, $offset = 0) {
    $steps = spaanmelden_form_lid_form_steps();
    $_steps = array_keys($steps);

    foreach ($_steps as $step_id => $step) {
        if($step == $current_step) {
            if(!empty($_steps[$step_id + $offset])) {
                return $_steps[$step_id + $offset];
            }

            return false;
        }
    }

    return false;
}

function spaanmelden_form_lid_form_personal_information($form, &$form_state) {
    $form['personal_information'] = array(
        '#type'  => 'fieldset',
        '#title' => 'Persoonsgegevens',
        '#tree'  => true,
    );

    $form['personal_information']['initials'] = array(
        '#title'         => 'Voorletters',
        '#type'          => 'textfield',
        '#default_value' => _spaanmelden_form_default_value($form_state, array('personal_information', 'initials'), ''),
        '#required'      => true,
    );

    $form['personal_information']['firstname'] = array(
        '#title'         => 'Voornaam',
        '#type'          => 'textfield',
        '#default_value' => _spaanmelden_form_default_value($form_state, array('personal_information', 'firstname'), ''),
        '#required'      => true,
    );

    $form['personal_information']['middlename'] = array(
        '#title'         => 'Tussenvoegsel',
        '#type'          => 'textfield',
        '#default_value' => _spaanmelden_form_default_value($form_state, array('personal_information', 'middlename'), ''),
        '#required'      => false,
    );

    $form['personal_information']['lastname'] = array(
        '#title'         => 'Achternaam',
        '#type'          => 'textfield',
        '#default_value' => _spaanmelden_form_default_value($form_state, array('personal_information', 'lastname'), ''),
        '#required'      => true,
    );

    $form['personal_information']['gender'] = array(
        '#title'         => 'Geslacht',
        '#type'          => 'radios',
        '#options'       => array('m' => 'Man', 'v' => 'Vrouw', 'a' => 'Anders'),
        '#default_value' => _spaanmelden_form_default_value($form_state, array('personal_information', 'gender'), null),
        '#required'      => true,
    );

    $form['personal_information']['birthdate'] = array(
        '#title'           => 'Geboortedatum',
        '#type'            => 'date_select',
        '#date_format'     => 'd-m-Y',
        '#date_year_range' => '-100:-15',
        '#default_value'   => _spaanmelden_form_default_value($form_state, array('personal_information', 'birthdate'), ''),
        '#required'        => true,
        '#ajax'            => array(
            'wrapper'  => 'spaanmelden_form',
            'callback' => 'spaanmelden_form_lid_form_ajax_callback',
            'progress' => 'none',
        ),
    );

    $lt_27 = _spaanmelden_form_lid_lt_27($form_state);

    $form['rood'] = array(
        '#type'        => 'fieldset',
        '#title'       => 'Word ook lid van ROOD!',
        '#tree'        => true,
        '#access'      => $lt_27,
        '#attributes'  => array('class' => array('doemee-lid-worden-rood')),
        '#description' => 'Je bent jonger dan 28 jaar en kunt ook meteen lid worden van ROOD, jong in de SP. ROOD-lid zijn kost je niets extra\'s en je krijgt bovendien een ROOD-shirt als welkomstcadeau!',
    );

    $form['rood']['rood'] = array(
        '#title'         => 'Lid worden van ROOD',
        '#type'          => 'radios',
        '#options'       => array(
            1 => 'Ja, ik wil ook lid worden van ROOD, Jong in de SP!',
            0 => 'Nee, ik wil alleen lid worden van de SP'
        ),
        '#default_value' => _spaanmelden_form_default_value($form_state, array('rood', 'rood'), null),
    );

    $form['contact'] = array(
        '#type'  => 'fieldset',
        '#title' => 'Contactgegevens',
        '#tree'  => true,
    );

    $form['contact']['email'] = array(
        '#title'         => 'E-mailadres',
        '#type'          => 'emailfield',
        '#default_value' => _spaanmelden_form_default_value($form_state, array('contact', 'email'), array()),
        '#required'      => true,
    );

    $form['contact']['email_confirm'] = array(
        '#title'         => 'Bevestig je e-mailadres',
        '#type'          => 'emailfield',
        '#default_value' => _spaanmelden_form_default_value($form_state, array('contact', 'email_confirm'), array()),
        '#required'      => true,
    );

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
    if(!empty($form_state['input']['address']['country']))
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
            'callback' => 'spaanmelden_form_lid_form_ajax_callback',
            'progress' => 'none',
        ),
    );

    $zipcode = _spaanmelden_form_default_value($form_state, array('address', 'zipcode'), '');
    if(!empty($form_state['input']['address']['zipcode']))
        $zipcode = $form_state['input']['address']['zipcode'];
    $form['address']['zipcode'] = array(
        '#title'         => 'Postcode',
        '#type'          => 'textfield',
        '#default_value' => $zipcode,
        '#required'      => true,
        '#attributes'    => array('style' => 'float: none !important;'),
        '#ajax'          => array(
            'wrapper'  => 'spaanmelden_form',
            'callback' => 'spaanmelden_form_lid_form_ajax_callback',
            'progress' => 'none',
        ),
    );

    $housenumber = _spaanmelden_form_default_value($form_state, array('address', 'housenumber'), '');
    if(!empty($form_state['input']['address']['housenumber']))
        $housenumber = $form_state['input']['address']['housenumber'];
    $form['address']['housenumber'] = array(
        '#title'         => 'Huisnummer',
        '#type'          => 'textfield',
        '#default_value' => $housenumber,
        '#required'      => true,
        '#attributes'    => array('style' => 'float: none !important;'),
        '#ajax'          => array(
            'wrapper'  => 'spaanmelden_form',
            'callback' => 'spaanmelden_form_lid_form_ajax_callback',
            'progress' => 'none',
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
        '#markup' => 'Uw straat- en plaatsnaam worden automatisch ingevuld.',
        '#suffix' => '</div>',
    );

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

function spaanmelden_form_lid_form_personal_information_tables(&$info, &$form_state) {
    $form = spaanmelden_form_lid_form_personal_information(array(), $form_state);

    foreach (element_children($form) as $fieldset_key) {
        $fieldset = $form[$fieldset_key];
        if($fieldset['#type'] != 'fieldset' || (isset($fieldset['#access']) && empty($fieldset['#access']))) {
            continue;
        }

        $info[check_plain($fieldset['#title'])] = _spaanmelden_fieldset_table($fieldset);
    }
}

function spaanmelden_form_lid_form_personal_information_submit($form, &$form_state) {
    if(!isset($form_state['storage']['data'])) {
        $form_state['storage']['data'] = array();
    }

    foreach (array('personal_information', 'contact', 'address', 'rood') as $key) {
        $form_state['storage']['data'][$key] = $form_state['values'][$key];
    }

    $lt_27 = _spaanmelden_form_lid_lt_27($form_state);
    if(!$lt_27 && isset($form_state['storage']['data']['rood'])) {
        unset($form_state['storage']['data']['rood']);
    }
}

function spaanmelden_form_lid_form_personal_information_validate($form, &$form_state) {
    $housenumber = drupal_array_get_nested_value($form_state['values'], array('address', 'housenumber'));
    if(!empty($housenumber)) {
        $housenumber = preg_replace('/[^0-9]/', '', $housenumber);
        $form_state['input']['address']['housenumber'] = $housenumber;
        $form_state['values']['address']['housenumber'] = $housenumber;

        if(empty($housenumber) || $housenumber < 0) {
            form_set_error('address][housenumber', 'Er is geen geldig huisnummer ingevoerd.');
        }
    }

    $zipcode = drupal_array_get_nested_value($form_state['values'], array('address', 'zipcode'));
    $country = drupal_array_get_nested_value($form_state['values'], array('address', 'country'));
    if(!preg_match('/^[1-9][0-9]{3}\s?([a-zA-Z]{2})?$/i', $zipcode) && $country == 'NL') {
        form_set_error('address][zipcode', 'Er is geen geldige postcode ingevoerd.');
    }

    $phone = drupal_array_get_nested_value($form_state['values'], array('contact', 'phone'));
    if(!empty($phone)) {
        $phone = preg_replace('/[^0-9]/', '', $phone);
        if(empty($phone) || strlen($phone) < 10) {
            form_set_error('contact][phone', 'Er is geen geldig telefoonnummer ingevoerd.');
        }
    }

    $phone_mobile = drupal_array_get_nested_value($form_state['values'], array('contact', 'phone_mobile'));
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

function spaanmelden_form_lid_form_donation($form, &$form_state) {
    $form['payment'] = array(
        '#type'  => 'fieldset',
        '#title' => 'Betalingsgegevens',
        '#tree'  => true,
    );

    $form['payment']['contribution'] = array(
        '#title'         => 'Bijdrage per kwartaal',
        '#type'          => 'radios',
        '#options'       => array('500'   => '&euro; 5,00 (minimum)', '750' => '&euro; 7,50',
                                  '1000'  => '&euro; 10,00', '1250' => '&euro; 12,50', '1500' => '&euro; 15,00',
                                  'other' => 'Anders, namelijk:'),
        '#default_value' => _spaanmelden_form_default_value($form_state, array('payment', 'contribution'), 500),
        '#required'      => true,
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
    );

    $form['payment']['payment_method'] = array(
        '#title'         => 'Ik betaal met',
        '#type'          => 'radios',
        '#options'       => array('incasso' => 'Automatische incasso', 'acceptgiro' => 'Acceptgiro (1 euro per kwartaal administratiekosten)'),
        '#default_value' => _spaanmelden_form_default_value($form_state, array('payment', 'payment_method'), null),
        '#required'      => true,
        '#ajax'          => array(
            'wrapper'  => 'spaanmelden_form',
            'callback' => 'spaanmelden_form_lid_form_ajax_callback',
            'progress' => 'none',
        ),
    );

    $form['payment']['bank_account'] = array(
        '#title'         => 'Rekeningnummer (IBAN)',
        '#type'          => 'textfield',
        '#default_value' => _spaanmelden_form_default_value($form_state, array('payment', 'bank_account'), ''),
        '#required'      => true,
        '#access'        => _spaanmelden_form_is_incasso($form_state),
        '#description'   => '<p>Vind uw IBAN nummer op de site <a href="http://www.openiban.nl/" target="_blank">OpenIBAN</a></p>',
    );

    return $form;
}

function spaanmelden_form_lid_form_donation_tables(&$info, &$form_state) {
    $form = spaanmelden_form_lid_form_donation(array(), $form_state);

    foreach (element_children($form) as $fieldset_key) {
        $fieldset = $form[$fieldset_key];
        if($fieldset['#type'] != 'fieldset' || (isset($fieldset['#access']) && empty($fieldset['#access']))) {
            continue;
        }

        $rows = array();

        $contribution = _spaanmelden_form_default_value($form_state, array('payment', 'contribution'), 500);
        if($contribution == 'other') {
            $contribution = floatval(_spaanmelden_form_default_value($form_state, array('payment', 'contribution_other'), ''));
        } else {
            $contribution = intval($contribution) / 100.0;
        }

        $rows['Bijdrage per kwartaal'] = '&euro; ' . number_format($contribution, 2, ',', '.');
        $rows = array_merge($rows, _spaanmelden_fieldset_table($fieldset, array('bank_account', 'payment_method')));
        $info[check_plain($fieldset['#title'])] = $rows;
    }
}

function spaanmelden_form_lid_form_donation_review(&$info, &$form_state) {
    $form = spaanmelden_form_lid_form_donation(array(), $form_state);

    $contribution = _spaanmelden_form_default_value($form_state, array('payment', 'contribution'), 500);
    if($contribution == 'other') {
        $contribution = floatval(_spaanmelden_form_default_value($form_state, array('payment', 'contribution_other'), ''));
    } else {
        $contribution = intval($contribution) / 100.0;
    }

    $rood = _spaanmelden_form_default_value($form_state, array('rood', 'rood'));
    $description = _spaanmelden_membership_description($rood ? 'lid_sprood' : 'lid_sp');
    $info['payment'] = array(
        '#markup' => '<p class="payment-info">Ik word ' . $description . '. Ik betaal per kwartaal &euro; ' . number_format($contribution, 2, ',', '.') . ' aan contributie.</p><p>Indien je per incasso betaalt, ontvang je twee weken voor de eerste afschrijving via internetbankieren bericht wanneer de contributie wordt afgeschreven.</p>',
        '#weight' => 100,
    );
}

function spaanmelden_form_lid_form_donation_validate($form, &$form_state) {
    $contribution = drupal_array_get_nested_value($form_state['values'], array('payment', 'contribution'));
    if($contribution == 'other') {
        $contribution_other = drupal_array_get_nested_value($form_state['values'], array('payment', 'contribution_other'));
        if(empty($contribution_other)) {
            form_set_error('payment][contribution_other', 'Vul alsjeblieft een bedrag in dat je per kwartaal wil bijdragen.');
        } elseif(floatval($contribution_other) < 5) {
            form_set_error('payment][contribution_other', 'De minimale kwartaalbijdrage is &euro; 5.');
        }
    }

	$payment_method = drupal_array_get_nested_value($form_state['values'], array('payment', 'payment_method'));
    $bank_account = drupal_array_get_nested_value($form_state['values'], array('payment', 'bank_account'));
    if($payment_method == 'incasso' && !_spaanmelden_check_iban($bank_account)) {
        form_set_error('payment][bank_account', 'Er is geen geldig rekeningnummer ingevoerd. U dient een IBAN nummer in te voeren.');
    }
}

function spaanmelden_form_lid_form_donation_submit($form, &$form_state) {
    if(!isset($form_state['storage']['data'])) {
        $form_state['storage']['data'] = array();
    }

    foreach (array('payment') as $key) {
        $form_state['storage']['data'][$key] = $form_state['values'][$key];
    }
}

function spaanmelden_form_lid_form_gift($form, &$form_state) {
    $form['welcome_present'] = array(
        '#type'  => 'fieldset',
        '#title' => 'Welkomstcadeau',
        '#tree'  => true,
    );

    $form['welcome_present']['intro'] = array(
        '#title'  => 'Welkomstcadeau',
        '#type'   => 'item',
        '#markup' => '<p>Kies uit één van de onderstaande welkomstcadeaus.</p>',
    );

    $form['welcome_present']['normal'] = array(
        '#title'         => 'Welkomstcadeau SP',
        '#type'          => 'select',
        '#options'       => _spaanmelden_get_gifts('lid_sp'),
        '#default_value' => _spaanmelden_form_default_value($form_state, array('welcome_present', 'normal'), ''),
        '#required'      => true,
    );

    $visible = false;
    if(!empty($form_state['storage']['data']['rood']['rood'])) {
        $visible = true;
    }

    $form['welcome_present']['rood'] = array(
        '#title'         => 'Welkomstcadeau ROOD',
        '#type'          => 'select',
        '#options'       => _spaanmelden_get_gifts('lid_rood'),
        '#default_value' => _spaanmelden_form_default_value($form_state, array('welcome_present', 'rood'), ''),
        '#required'      => true,
        '#access'        => $visible,
    );

    return $form;
}

function spaanmelden_form_lid_form_gift_tables(&$info, &$form_state) {
    $form = spaanmelden_form_lid_form_gift(array(), $form_state);

    foreach (element_children($form) as $fieldset_key) {
        $fieldset = $form[$fieldset_key];
        if($fieldset['#type'] != 'fieldset' || (isset($fieldset['#access']) && empty($fieldset['#access']))) {
            continue;
        }

        $info[check_plain($fieldset['#title'])] = _spaanmelden_fieldset_table($fieldset);
    }
}

function spaanmelden_form_lid_form_gift_submit($form, &$form_state) {
    if(!isset($form_state['storage']['data'])) {
        $form_state['storage']['data'] = array();
    }

    foreach (array('welcome_present') as $key) {
        $form_state['storage']['data'][$key] = $form_state['values'][$key];
    }
}

function spaanmelden_form_lid_form_review($form, &$form_state) {
    $steps = spaanmelden_form_lid_form_steps();
    $form['info'] = array();

    foreach ($steps as $key => $info) {
        $callback = $info['callback'] . '_review';

        if(function_exists($callback)) {
            $callback($form['info'], $form_state);
        }
    }

    return $form;
}

function spaanmelden_form_lid_form_review_review(&$info, &$form_state) {
    $info['tables'] = _spaanmelden_format_tables('lid', $form_state);
    $info['#weight'] = 10;
}

function _spaanmelden_form_lid_lt_27(&$form_state) {
    $birthdate = _spaanmelden_form_default_value($form_state, array('personal_information', 'birthdate'), '');
    $birthdate = $birthdate ? (array_combine(array('year', 'month', 'day'), explode('-', $birthdate))) : array();
    if(!empty($form_state['input']['personal_information']['birthdate'])) {
        $birthdate = array_map('intval', $form_state['input']['personal_information']['birthdate']);
    }
    if(!$birthdate || count($birthdate) == 0)
        return false;

    $birthdate = new \DateTime($birthdate['year'] . '-' . $birthdate['month'] . '-' . $birthdate['day']);
    $now = new \DateTime;

    $interval = $birthdate->diff($now);
    return ($interval->y <= 27);
}
