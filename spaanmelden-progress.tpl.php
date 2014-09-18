<div class="spaanmelden-progress" id="spaanmelden-progress-indicator">
  <ul>
    <?php foreach (array_keys($steps) as $i => $key):
      $attributes = array();
      if ($key == $active_step) {
        $attributes['class'] = array('active');
        ?>
      <li <?php print drupal_attributes($attributes); ?>>
        <h2 class="step-title">Stap <?php print $i+1; ?> van <?php print count($steps)?>: <span class="step-text"><?php print check_plain($steps[$key]['title']); ?></span></h2>
      </li>
    <?php
      }
    ?>
    <?php endforeach; ?>
  </ul>
</div>
