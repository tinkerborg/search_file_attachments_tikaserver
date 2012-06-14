<?php

/**
 * @file
 * Admin settings
 */

/**
 * Page callback to show the settings for the attachments.
 */
function search_file_attachments_settings_form() {
  $form = array();

  // Apache Tika configuration.
  $form['tika'] = array(
    '#type' => 'fieldset',
    '#title' => t('Apache Tika'),
  );
  $form['tika']['search_file_attachments_tika_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Tika directory path:'),
    '#default_value' => variable_get('search_file_attachments_tika_path', ''),
    '#description' => t('The full path to tika directory.'),
  );
  $form['tika']['search_file_attachments_tika_jar'] = array(
    '#type' => 'textfield',
    '#title' => t('Tika jar file:'),
    '#default_value' => variable_get('search_file_attachments_tika_jar', 'tika-app-1.1.jar'),
    '#description' => t('The name of the tika CLI application jar file, e.g. tika-app-1.1.jar.'),
  );

  // File settings.
  $form['files'] = array(
    '#type' => 'fieldset',
    '#title' => t('File settings'),
  );
  $form['files']['exclude_formats'] = array(
    '#type' => 'textfield',
    '#title' => t('Exclude file extensions'),
    '#default_value' => variable_get('search_file_attachments_exclude_formats', 'aif, art, avi, bmp, gif, ico, mov, oga, ogv, png, psd, ra, ram, rgb'),
  );

  return system_settings_form($form);
}

/**
 * Validation handler for the settings form.
 */
function search_file_attachments_settings_form_validate($form, &$form_state) {
  if (empty($form_state['values']['search_file_attachments_tika_jar'])) {
    form_set_error('search_file_attachments_tika_jar', t('Tika jar is mandatory.'));
  }
  $path = realpath($form_state['values']['search_file_attachments_tika_path']);
  if (!file_exists($path . '/' . $form_state['values']['search_file_attachments_tika_jar'])) {
    form_set_error('search_file_attachments_tika_path', t('Tika jar file not found at this path.'));
  }
}
