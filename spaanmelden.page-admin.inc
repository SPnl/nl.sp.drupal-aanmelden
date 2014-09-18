<?php

function spaanmelden_admin_settings_form() {

    $form = array();

    $form['spaanmelden_description_civicrm'] = array(
        '#markup' => '<h3>Instellingen</h3><p>Deze module maakt drie aanmeldformulieren (<a href="' . url('doe-mee/aanmelden/lid') . '">voor leden</a>, <a href="' . url('doe-mee/aanmelden/tribune') . '">Tribune-</a> en <a href="' . url('doe-mee/aanmelden/spanning') . '">Spanning-abonnees</a>) aan die geintegreerd zijn met CiviCRM.<br />Stel hieronder de server en API-instellingen in voor deze integratie.</p>',
    );

    $form['spaanmelden_civicrm_server'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Server',
        '#default_value' => variable_get('spaanmelden_civicrm_server'),
        '#description'   => 'URL van de CiviCRM-installatie.',
        '#required'      => true,
    );

    $form['spaanmelden_civicrm_path'] = array(
        '#type'          => 'textfield',
        '#title'         => 'API-pad',
        '#default_value' => variable_get('spaanmelden_civicrm_path'),
        '#description'   => 'Pad naar de API (bijvoorbeeld: sites/test/modules/civicrm/extern/rest.php)',
        '#required'      => true,
    );

    $form['spaanmelden_civicrm_key'] = array(
        '#type'          => 'textfield',
        '#title'         => 'Key',
        '#default_value' => variable_get('spaanmelden_civicrm_key'),
        '#description'   => 'CiviCRM-key (uit civicrm.settings.php).',
        '#required'      => true,
    );

    $form['spaanmelden_civicrm_userkey'] = array(
        '#type'          => 'textfield',
        '#title'         => 'User-key',
        '#default_value' => variable_get('spaanmelden_civicrm_userkey'),
        '#description'   => 'CiviCRM API-key (uit civicrm_contact).',
        '#required'      => true,
    );

    $form['spaanmelden_description_text'] = array(
        '#markup' => '<br /><h3>Paginatekst</h3><p>De onderstaande teksten worden bovenaan de formulieren getoond.</p>',
    );

    $lid_text = variable_get('spaanmelden_text_lid', array('value' => '', 'format' => null));
    $form['spaanmelden_text_lid'] = array(
        '#type'          => 'text_format',
        '#title'         => 'Tekst aanmelden als lid',
        '#default_value' => $lid_text['value'],
        '#format'        => $lid_text['format'],
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
            'name'      => ($s->firstname ? $s->firstname : $s->initials) . ' ' . ($s->middlename ? $s->minddlename . ' ' : '') . $s->lastname,
            'type'      => _spaanmelden_membership_description($s->membership_type, true),
            'created'   => date('d-m-Y H:i', $s->created),
            'verified'  => ($s->verified ? date('d-m-Y H:i', $s->verified) : '-'),
            'submitted' => ($s->errormsg ? date('d-m-Y H:i', $s->submitted) . ' - <span style="color:#c00;font-weight:bold;">ERROR</span>' : ($s->submitted ? date('d-m-Y H:i', $s->submitted) . ' - <span style="color:#0c0;font-weight:bold;">OK</span>' : '-')),
            'options'   => l('Details', 'admin/config/sp/aanmelden/detail/' . $s->id),
        );
    }

    if(!empty($data)) {
        $output['data'] = array(
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
        foreach ($s as $key => $value) {

            switch ($key) {
                case 'hash':
                    continue 2;
                    break;

                case 'created':
                case 'verified':
                case 'reminded':
                case 'submitted':
                    if($value > 0)
                        $value = date('d-m-Y H:i:s', $value);
                    else
                        $value = '-';
                    break;

                case 'membership_type':
                    $value = _spaanmelden_membership_description($value, true);
                    break;

                case 'errormsg':
                    if($value != '') {
                        $value = '<span style="color:#f00;font-weight:bold;">' . $value . '</span>';
                    } else {
                        continue 2;
                    }
                    break;
            }

            $rows[] = array(
                'key'   => $key,
                'value' => $value,
            );
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