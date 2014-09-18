<?php if($txt):
    print $txt['value'];
endif; ?>

<div class="content-wrap">
    <?php print drupal_render_children($form); ?>
</div>