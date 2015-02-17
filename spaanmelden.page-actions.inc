<?php

function spaanmelden_page_thanks() {
    $name = base64_decode($_GET['name']);

    if(!drupal_valid_token($_GET['token'], $name, true)) {
        return MENU_ACCESS_DENIED;
    }

    drupal_set_title($name . ', bedankt!');

    $ret = '<div class="lid-worden-bedankt content">';
    $descr = _spaanmelden_membership_description($_GET['membership_type']);
    $ret .= '<p>Bedankt voor uw aanmelding als ' . $descr . '. ';
    if(!in_array($_GET['membership_type'], array('tribune', 'spanning')))
        $ret .= 'Goed dat u ons wilt steunen in de strijd voor een beter Nederland!<br />';
    $ret .= 'U ontvangt straks per e-mail een overzicht van de door u opgegeven gegevens.<br />
            Let op! De gegevens moet u nog wel even via een link in die e-mail bevestigen.</p>'
        . _spaanmelden_get_qline() . '</div>';
    return $ret;
}

function spaanmelden_page_error() {
    return '<div class="lid-worden-bedankt content"><h1>Aanmelding niet verwerkt</h1><p>Excuus, maar het is
     niet gelukt om je aanmelding goed te verwerken. Probeer het later nog eens, of neem even contact met ons
      op.' . _spaanmelden_get_qline() . '</p></div>';
}

function spaanmelden_page_validate($hash) {

    $q = db_select('sp_aanmeldingen', 's')
        ->fields('s')
        ->condition('hash', $hash, '=')
        ->execute();
    if($q) {
        $record = $q->fetchAssoc();

        $exec = db_update('sp_aanmeldingen')
            ->fields(array(
                'verified' => time()
            ))
            ->condition('hash', $hash, '=')
            ->execute();

        if($exec) {
            return '<div class="lid-worden-bedankt content"><h1>Aanmelding bevestigd</h1><p>Bedankt voor
                het bevestigen van uw gegevens. Uw aanmelding wordt zo spoedig mogelijk verwerkt door onze
                 ledenadministratie.</p>' . _spaanmelden_get_qline() . '</div>';
        }
    }

    return '<div class="lid-worden-bedankt content"><h1>Aanmelding niet bevestigd</h1><p>Er is een fout
     opgetreden bij het verwerken van uw aanvraag. Controleer of de link die u gevolgd hebt klopt, en kopieer
      deze eventueel handmatig uit de e-mail.</p>' . _spaanmelden_get_qline() . '</div>';

}

// Postcode-lookup voor aanmeldformulieren - voor nu afgehandeld via Drupal Ajax
/*
function spaanmelden_page_postcode() {
    if($_POST['zipcode'] && $_POST['housenumber'])
        $data = _spaanmelden_postcode_lookup($_POST['zipcode'], $_POST['housenumber']);
    else
        $data = array();

    drupal_json_output($data);
}
*/
