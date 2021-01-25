<?php

/**
 * @file
 * Admin settings
 */

require_once dirname(__FILE__) . '/search_file_attachments.inc';

/**
 * Page callback to show the settings for the attachments.
 */
function search_file_attachments_settings_form() {
  $form = array();

  $conf_dir = 'private://search_file_attachments';

  $form['#submit'][] = 'search_file_attachments_settings_submit';

  // Apache Tika configuration.
  $form['tika'] = array(
    '#type' => 'fieldset',
    '#title' => t('Apache Tika'),
  );

  $form['tika']['search_file_attachments_tika_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Tika Server URL:'),
    '#default_value' => search_file_attachments_tika_url(),
    '#description' => t('The base URL of the Tika server.'),
  );

  // Advanced settings.
  $form['tika']['tls'] = array(
    '#type' => 'fieldset',
    '#title' => t('TLS Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['tika']['tls']['search_file_attachments_tls_cert'] = [
    '#type' => 'managed_file',
    '#title' => t('Client Certificate (PEM)'),
    '#upload_location' => $conf_dir,
    '#default_value' => variable_get('search_file_attachments_tls_cert'),

    '#upload_validators' => [
      'file_validate_extensions' => ['crt', 'pem'],
    ],
  ];

  $form['tika']['tls']['search_file_attachments_tls_key'] = [
    '#type' => 'managed_file',
    '#title' => t('Client Private Key'),
    '#upload_location' => $conf_dir,
    '#default_value' => variable_get('search_file_attachments_tls_key'),

    '#upload_validators' => [
      'file_validate_extensions' => ['key', 'pem'],
    ],
  ];

  $form['tika']['tls']['search_file_attachments_tls_ca_cert'] = [
    '#type' => 'managed_file',
    '#title' => t('TLS CA Certificate'),
    '#upload_location' => $conf_dir,
    '#default_value' => variable_get('search_file_attachments_tls_ca_cert'),

    '#upload_validators' => [
      'file_validate_extensions' => ['crt', 'pem'],
    ],
  ];


  // File settings.
  $form['files'] = array(
    '#type' => 'fieldset',
    '#title' => t('File settings'),
  );

  $form['files']['search_file_attachments_include_extensions'] = array(
    '#type' => 'textfield',
    '#title' => t('Include file extensions or mimetypes'),
    '#description' => t('A comma-separated list of file extensions or mimetypes that will be included to the file search index.'),
    '#default_value' => search_file_extensions_included_extensions(),
  );

  // Advanced settings.
  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['advanced']['search_file_attachments_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Activate Debugging'),
    '#description' => t('Activate this option only for development and not on production sites.'),
    '#default_value' => variable_get('search_file_attachments_debug', FALSE),
  );

  return system_settings_form($form);
}

/**
 * Validation handler for the settings form.
 */
function search_file_attachments_settings_form_validate($form, &$form_state) {
  #if (empty($form_state['values']['search_file_attachments_tika_jar'])) {
  #  form_set_error('search_file_attachments_tika_jar', t('Tika jar is mandatory.'));
  #}
  #$path = realpath($form_state['values']['search_file_attachments_tika_path']);
  #if (!file_exists($path . '/' . $form_state['values']['search_file_attachments_tika_jar'])) {
  #  form_set_error('search_file_attachments_tika_path', t('Tika jar file not found at this path.'));
  #}
}

/**
 * Submit handler for the settings form.
 */
function search_file_attachments_settings_submit($form, &$form_state) {
  foreach (array_filter(
    array_intersect_key($form_state['values'],
      array_flip(
        preg_grep('/^search_file_attachments_tls/', array_keys($form_state['values']))
      )
    ), function($fid) { return $fid > 0; }
  ) as &$fid) {
   $file = file_load($fid);
   $file->status = FILE_STATUS_PERMANENT;
   file_save($file);
   file_usage_add($file, 'search_file_attachments', 'tls', $fid);
  }
}
