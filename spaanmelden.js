(function ($) {

    /* Code uit Drupal-module Hoppinger */

    Drupal.ajax.prototype.commands.scrollToTop = function (ajax, response, status) {
        var formOffset = $('#spaanmelden_form').offset();
        $('html, body').animate({scrollTop: formOffset.top}, 400, 'easeOutQuad');
    };

    Drupal.ajax.prototype.commands.scrollToWrapper = function (ajax, response, status) {
        $('html, body').animate({scrollTop: $(ajax.wrapper, ajax.form).offset().top - 100}, 400, 'easeOutQuad');
    };

    Drupal.ajax.prototype.commands.focusElement = function (ajax, response, status) {
        if (response && response.element) {
            setTimeout(function () {
                var next = $('.' + response.element).next('.form-item');
                if (next) {
                    var field = next.find('input, select').first();
                    if (field)
                        field.focus();
                }
            }, 100);
        }
    }

}(jQuery));
