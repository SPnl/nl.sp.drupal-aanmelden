<?php

function spaanmelden_admin_texts_form() {

    $form = array();

    $form['spaanmelden_description_text'] = array(
        '#markup' => '<h3>Teksten</h3><p>Deze module maakt drie aanmeldformulieren (<a href="' . url('aanmelden/lid') . '">voor leden</a>, <a href="' . url('aanmelden/tribune') . '">Tribune-</a> en <a href="' . url('aanmelden/spanning') . '">Spanning-abonnees</a>) aan die geintegreerd zijn met CiviCRM.<br />De onderstaande teksten worden bovenaan de formulieren getoond.</p>',
    );

    $lid_text = variable_get('spaanmelden_text_lid', array('value' => '', 'format' => null));
    $form['spaanmelden_text_lid'] = array(
        '#type'          => 'text_format',
        '#title'         => 'Tekst aanmelden als lid',
        '#default_value' => $lid_text['value'],
        '#format'        => $lid_text['format'],
        '#required'      => false,
    );

    $rood_text = variable_get('spaanmelden_text_rood', array('value' => '', 'format' => null));
    $form['spaanmelden_text_rood'] = array(
        '#type'          => 'text_format',
        '#title'         => 'Tekst aanmelden als ROOD-lid',
        '#default_value' => $rood_text['value'],
        '#format'        => $rood_text['format'],
        '#required'      => false,
    );

    $tribune_text = variable_get('spaanmelden_text_tribune', array('value' => '', 'format' => null));
    $form['spaanmelden_text_tribune'] = array(
        '#type'          => 'text_format',
        '#title'         => 'Tekst aanmelden als Tribune-abonnee',
        '#default_value' => $tribune_text['value'],
        '#format'        => $tribune_text['format'],
        '#required'      => false,
    );

    $spanning_text = variable_get('spaanmelden_text_spanning', array('value' => '', 'format' => null));
    $form['spaanmelden_text_spanning'] = array(
        '#type'          => 'text_format',
        '#title'         => 'Tekst aanmelden als SPanning-abonnee',
        '#default_value' => $spanning_text['value'],
        '#format'        => $spanning_text['format'],
        '#required'      => false,
    );

    $intern_text = variable_get('spaanmelden_text_intern', array('value' => '', 'format' => null));
    $form['spaanmelden_text_intern'] = array(
        '#type'          => 'text_format',
        '#title'         => 'Tekst aanmelden intern formulier',
        '#default_value' => $intern_text['value'],
        '#format'        => $intern_text['format'],
        '#required'      => false,
    );

    return system_settings_form($form);
}

function spaanmelden_admin_settings_form() {

    $form = array();

    $form['spaanmelden_description_civicrm'] = array(
        '#markup' => '<h3>Instellingen</h3><p>Configureer hieronder de integratie met CiviCRM.</p>',
    );

    $form['spaanmelden_id_lid_sp'] = array(
        '#type'          => 'textfield',
        '#title'         => 'ID lidmaatschap SP',
        '#default_value' => variable_get('spaanmelden_id_lid_sp', 1),
        '#required'      => true,
    );

	$form['spaanmelden_id_fintype_sp'] = array(
		'#type'          => 'textfield',
		'#title'         => 'ID financieel type SP',
		'#default_value' => variable_get('spaanmelden_id_fintype_sp', 6),
		'#required'      => true,
	);

    $form['spaanmelden_id_lid_sprood'] = array(
        '#type'          => 'textfield',
        '#title'         => 'ID lidmaatschap SP en ROOD',
        '#default_value' => variable_get('spaanmelden_id_lid_sprood', 2),
        '#required'      => true,
    );

	$form['spaanmelden_id_fintype_sprood'] = array(
		'#type'          => 'textfield',
		'#title'         => 'ID financieel type SP en ROOD',
		'#default_value' => variable_get('spaanmelden_id_fintype_sp', 7),
		'#required'      => true,
	);

    $form['spaanmelden_id_lid_rood'] = array(
        '#type'          => 'textfield',
        '#title'         => 'ID lidmaatschap ROOD',
        '#default_value' => variable_get('spaanmelden_id_lid_rood', 3),
        '#required'      => true,
    );

    $form['spaanmelden_id_fintype_rood'] = array(
        '#type'          => 'textfield',
        '#title'         => 'ID financieel type ROOD',
        '#default_value' => variable_get('spaanmelden_id_fintype_rood', 5),
        '#required'      => true,
    );

    $form['spaanmelden_id_abo_tribune'] = array(
        '#type'          => 'textfield',
        '#title'         => 'ID abonnement Tribune',
        '#default_value' => variable_get('spaanmelden_id_abo_tribune', 4),
        '#required'      => true,
    );

	$form['spaanmelden_id_fintype_tribune'] = array(
		'#type'          => 'textfield',
		'#title'         => 'ID financieel type Tribune',
		'#default_value' => variable_get('spaanmelden_id_fintype_tribune', 10),
		'#required'      => true,
	);

    $form['spaanmelden_id_abo_spanning'] = array(
        '#type'          => 'textfield',
        '#title'         => 'ID abonnement Spanning',
        '#default_value' => variable_get('spaanmelden_id_abo_spanning', 10),
        '#required'      => true,
    );

	$form['spaanmelden_id_fintype_spanning'] = array(
		'#type'          => 'textfield',
		'#title'         => 'ID financieel type Spanning',
		'#default_value' => variable_get('spaanmelden_id_fintype_spanning', 8),
		'#required'      => true,
	);

    $form['spaanmelden_id_status'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Status-ID voor lidmaatschappen',
        '#default_value' => variable_get('spaanmelden_id_status', 5),
        '#required'      => true,
    );

	$form['spaanmelden_id_payment_incasso'] = array(
		'#type'          => 'textfield',
		'#title'         => 'ID betaalwijze incasso',
		'#default_value' => variable_get('spaanmelden_id_payment_incasso', 10),
		'#required'      => true,
	);

	$form['spaanmelden_id_payment_accept'] = array(
		'#type'          => 'textfield',
		'#title'         => 'ID betaalwijze acceptgiro',
		'#default_value' => variable_get('spaanmelden_id_payment_accept', 9),
		'#required'      => true,
	);

    $form['spaanmelden_id_activity'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Naam toe te voegen activiteit',
        '#default_value' => variable_get('spaanmelden_id_activity', 'Aanmelding via website'),
        '#required'      => true,
    );

    $form['spaanmelden_id_activity_contact'] = array(
        '#type'          => 'textfield',
        '#title'         => 'ID Contact voor activiteit',
        '#default_value' => variable_get('spaanmelden_id_activity_contact', 7),
        '#required'      => true,
    );

    $form['spaanmelden_id_custom_initials'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Voorletters',
        '#default_value' => variable_get('spaanmelden_id_custom_initials', 'custom_111'),
        '#required'      => true,
    );

    $form['spaanmelden_id_custom_gift_sp'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Veldnaam welkomstcadeau SP',
        '#default_value' => variable_get('spaanmelden_id_custom_gift_sp', 'custom_119'),
        '#required'      => true,
    );

    $form['spaanmelden_id_custom_gift_rood'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Veldnaam welkomstcadeau ROOD',
        '#default_value' => variable_get('spaanmelden_id_custom_gift_rood', 'custom_122'),
        '#required'      => true,
    );

    $form['spaanmelden_id_custom_gift_sp_received'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Veldnaam welkomstcadeau SP ontvangen',
        '#default_value' => variable_get('spaanmelden_id_custom_gift_sp_received', 'custom_120'),
        '#required'      => true,
    );

    $form['spaanmelden_id_custom_gift_rood_received'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Veldnaam welkomstcadeau ROOD ontvangen',
        '#default_value' => variable_get('spaanmelden_id_custom_gift_rood_received', 'custom_123'),
        '#required'      => true,
    );

    $form['spaanmelden_id_custom_source'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Veldnaam bron bij contact',
        '#default_value' => variable_get('spaanmelden_id_custom_source', 'custom_112'),
        '#required'      => true,
    );

    $form['spaanmelden_id_custom_source_membership'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Veldnaam bron bij lidmaatschap',
        '#default_value' => variable_get('spaanmelden_id_custom_source_membership', 'custom_114'),
        '#required'      => true,
    );

    $form['spaanmelden_id_custom_gezinslid'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Veldnaam gezinslid',
        '#default_value' => variable_get('spaanmelden_id_custom_gezinslid', 'custom_127'),
        '#required'      => true,
    );

    $form['spaanmelden_id_custom_welkomstbrief'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Veldnaam welkomstbrief',
        '#default_value' => variable_get('spaanmelden_id_custom_welkomstbrief', 'custom_125'),
        '#required'      => true,
    );

    $form['spaanmelden_id_custom_reden_lidmaatschap'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Veldnaam reden lidmaatschap',
        '#default_value' => variable_get('spaanmelden_id_custom_reden_lidmaatschap', 'custom_218'),
        '#required'      => true,
    );

    return system_settings_form($form);
}

function spaanmelden_view_submissions() {

    $header = array('id'        => 'ID',
                    'name'      => 'Naam',
                    'type'      => 'Type',
                    'created'   => 'Aangemeld',
                    'verified'  => 'Geverifieerd',
                    'submitted' => 'Toegevoegd',
                    'options'   => 'Opties',
    );
    $data = array();

    $submissions = db_select('sp_aanmeldingen', 's')->fields('s')->orderBy('created', 'DESC')->execute()->fetchAll();
    foreach ($submissions as $s) {

        $data[] = array(
            'id'        => $s->id,
            'name'      => ($s->firstname ? $s->firstname : $s->initials) . ' ' . ($s->middlename ? $s->middlename . ' ' : '') . $s->lastname,
            'type'      => _spaanmelden_membership_description($s->membership_type, true),
            'created'   => date('d-m-Y H:i', $s->created),
            'verified'  => ($s->verified ? date('d-m-Y H:i', $s->verified) : '-'),
            'submitted' => ($s->errormsg ? date('d-m-Y H:i', $s->submitted) . ' - <span style="color:#c00;font-weight:bold;">ERROR</span>' : ($s->submitted ? date('d-m-Y H:i', $s->submitted) . ' - <span style="color:#0c0;font-weight:bold;">' . ($s->contact_id ? $s->contact_id  : 'OK') . '</span>' : '-')),
            'options'   => l('Details', 'admin/config/sp/aanmelden/submission/' . $s->id),
        );
    }

    if(!empty($data)) {
        $output['data'] = array(
            '#prefix' => '<a href="' . url('admin/config/sp/aanmelden/submit') . '">Handmatig synchroniseren</a><br /><br />',
            '#theme'  => 'table',
            '#header' => $header,
            '#rows'   => $data,
        );
    } else {
        $output = 'Er zijn nog geen inschrijvingen aanwezig.';
    }

    return $output;
}

function spaanmelden_submission_details($id) {

    $header = array('key'   => 'Veld',
                    'value' => 'Waarde');
    $data = array();

    $s = db_select('sp_aanmeldingen', 's')
        ->fields('s')
        ->condition('s.id', $id)
        ->execute()
        ->fetchAssoc();

    if($s) {
        $rows = array();
        foreach ($s as $key => &$value) {

            list($key, $value) = _spaanmelden_friendly_field_data($key, $value);
            if($key)
                $rows[] = array('key' => $key, 'value' => $value);
        }

        $output['data'] = array(
            '#theme'  => 'table',
            '#header' => $header,
            '#rows'   => $rows,
        );
    } else {
        $output = 'Geen record gevonden met deze ID.';
    }

    return $output;
}

function spaanmelden_gifts_view($action = null, $id = null) {

    //Delete gift
    if($action == 'delete') {
        if(is_numeric($id)) {

            $chk = db_select('sp_aanmeldingen', 's')
                    ->condition(db_or()->condition('welcome_present_sp', $id)->condition('welcome_present_rood', $id))
                    ->countQuery()->execute()->fetchField();
            if($chk > 0) {
                drupal_set_message(t('Dit welkomstgeschenk is gekoppeld aan eerdere aanmeldingen, en is daarom niet verwijderd maar op inactief gezet.'));
                db_update('sp_aanmeldingen_gifts')
                    ->fields(array('show_website' => 0))
                    ->condition('id', $id)
                    ->execute();
            } else {
                $delete = db_delete('sp_aanmeldingen_gifts')
                  ->condition('id', $id)
                  ->execute();
                if ($delete) {
                    drupal_set_message(t('Welkomstgeschenk !1 is verwijderd.', array('!1' => $id)));
                }
                else {
                    drupal_set_message(t('Het welkomstgeschenk kon niet worden verwijderd. Controleer de link en database-instellingen.'), $type = 'error');
                }
            }
        }
    }

    // View gifts
    $header = array('id'              => 'ID',
                    'name'            => 'Naam',
                    'membership_type' => 'Lidmaatschap',
        'show_website' => 'Getoond op site',
                    'options'         => 'Opties',
    );
    $data = array();

    $gifts = db_select('sp_aanmeldingen_gifts', 'sg')->fields('sg')->orderBy('membership_type', 'DESC')->orderBy('position', 'ASC')->execute()->fetchAll();
    foreach ($gifts as $g) {

        $data[] = array(
            'id'      => $g->id,
            'name'    => $g->name,
            'type'    => _spaanmelden_membership_description($g->membership_type, true),
            'show_website' => ($g->show_website ? 'ja' : 'nee'),
            'options' =>
                l('Bewerken', 'admin/config/sp/aanmelden/gifts/edit/' . $g->id) . '&nbsp;&nbsp;&nbsp;' .
                l('Verwijderen', 'admin/config/sp/aanmelden/gifts/delete/' . $g->id),
        );
    }

    $addLink = '<a href="' . url('admin/config/sp/aanmelden/gifts/edit') . '">Nieuw welkomstgeschenk</a><br /><br />';
    if(!empty($data)) {
        $output['data'] = array(
            '#prefix' => $addLink,
            '#theme'  => 'table',
            '#header' => $header,
            '#rows'   => $data,
        );
    } else {
        $output['data'] = array(
            '#prefix' => $addLink . 'Er zijn nog geen welkomstcadeaus toegevoegd.',
            '#theme'  => 'table',
        );
    }

    return $output;
}

function spaanmelden_gifts_form($form = null, $form_state = null, $id = null) {

    //Begin add form
    $form = array();

    $form['add_gift'] = array(
        '#type'        => 'fieldset',
        '#title'       => t('Welkomstgeschenk'),
        '#description' => t('Kies een naam voor het welkomstgeschenk en voor welke lidmaatschappen het geschenk van toepassing is. De interne naam wordt doorgegeven aan CiviCRM.'),
        '#tree'        => true,
        '#parents'     => array('add_gift'),
    );
    $form['add_gift']['name'] = array(
        '#type'        => 'textfield',
        '#title'       => 'Naam',
        '#description' => 'Deze naam wordt getoond op het formulier.',
        '#required'    => true,
    );
    $form['add_gift']['internal_name'] = array(
        '#type'        => 'textfield',
        '#title'       => 'Interne naam',
        '#description' => 'Deze naam wordt doorgegeven aan CiviCRM.',
        '#required'    => true,
    );
    $form['add_gift']['membership_type'] = array(
        '#type'        => 'select',
        '#title'       => 'Lidmaatschap',
        '#options'     => array('lid_sp' => 'SP', 'lid_rood' => 'ROOD'),
        '#description' => 'Kies of dit cadeau voor SP of ROOD wordt getoond.',
        '#required'    => true,
    );
    $form['add_gift']['show_website'] = array(
        '#type'        => 'select',
        '#title'       => 'Tonen op website?',
        '#options'     => array(1 => 'ja', 0 => 'nee'),
        '#default_value' => 1,
        '#description' => 'Kies of dit cadeau op de website moet worden getoond. Op het interne formulier worden alle cadeaus getoond.',
        '#required'    => true,
    );

    $weightOptions = array();
    for ($i = -20; $i <= 20; $i++)
        $weightOptions[] = $i;
    $form['add_gift']['position'] = array(
        '#type'          => 'select',
        '#options'       => $weightOptions,
        '#title'         => t('Weight'),
        '#default_value' => 0,
        '#description'   => 'Optioneel. Hoe lager het gewicht, hoe eerder het geschenk wordt getoond.',
        '#required'      => false,
    );

    $form['submit'] = array(
        '#type'  => 'submit',
        '#value' => 'Toevoegen',
    );

    //Begin edit form
    if(!empty($id) && is_numeric($id)) {

        $default_values = db_select('sp_aanmeldingen_gifts', 'sg')->fields('sg')->condition('id', $id)->execute()->fetchObject();
        if(!$default_values) {
            drupal_set_message('Het welkomstgeschenk kon niet opgehaald worden uit de database. Controleer de link en de database-instellingen.');
            return $form;
        }

        //Alter above add form with default values.
        $form['add_gift']['name']['#default_value'] = $default_values->name;
        $form['add_gift']['internal_name']['#default_value'] = $default_values->internal_name;
        $form['add_gift']['membership_type']['#default_value'] = $default_values->membership_type;
        $form['add_gift']['position']['#default_value'] = $default_values->position;
        $form['add_gift']['show_website']['#default_value'] = $default_values->show_website;
        $form['edit_flag'] = array('#type' => 'hidden', '#value' => check_plain($id));
        $form['submit']['#value'] = 'Opslaan';
    }

    return $form;
}

function spaanmelden_gifts_form_validate($form, &$form_state) {
    if(!$form_state['values']['add_gift']['name'] ||
        !$form_state['values']['add_gift']['internal_name'] ||
        !$form_state['values']['add_gift']['membership_type']
    ) {
        form_set_error('add_gift', 'U hebt niet alle verplichte velden ingevuld.');
    }
}

function spaanmelden_gifts_form_submit($form, &$form_state) {

    if(isset($form_state['values']['edit_flag'])) {
        $edit_gift = db_update('sp_aanmeldingen_gifts')->fields(array(
            'name'            => $form_state['values']['add_gift']['name'],
            'internal_name'   => $form_state['values']['add_gift']['internal_name'],
            'membership_type' => $form_state['values']['add_gift']['membership_type'],
            'position'        => (int)$form_state['values']['add_gift']['position'],
            'show_website'    => (int)$form_state['values']['add_gift']['show_website'],
        ))->condition('id', (int)$form_state['values']['edit_flag'])->execute();

        if($edit_gift) {
            drupal_set_message('Het geschenk is opgeslagen.');
            drupal_goto('admin/config/sp/aanmelden/gifts');
        }
    } else {
        $add_gift = db_insert('sp_aanmeldingen_gifts')->fields(array(
            'name'            => $form_state['values']['add_gift']['name'],
            'internal_name'   => $form_state['values']['add_gift']['internal_name'],
            'membership_type' => $form_state['values']['add_gift']['membership_type'],
            'position'        => (int)$form_state['values']['add_gift']['position'],
            'show_website'    => (int)$form_state['values']['add_gift']['show_website'],
        ))->execute();
        if($add_gift) {
            drupal_set_message('Het geschenk is toegevoegd.');
            drupal_goto('admin/config/sp/aanmelden/gifts');
        } else {
            drupal_set_message('Het is niet gelukt het geschenk toe te voegen aan de database.', $type = 'error');
        }
    }
}

function spaanmelden_admin_submit_pending() {

    spaanmelden_submit_pending();

    drupal_set_message('Wachtende inschrijvingen zijn gesynchroniseerd met CiviCRM.');
    drupal_goto('admin/config/sp/aanmelden');
}
