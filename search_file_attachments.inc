<?php

/**
 * @file
 * Heler functions, to hold the .module file clean and smart.
 */

/**
 * Shutdown function to make sure we remember the last element processed.
 */
function search_file_attachments_shutdown() {
  global $last_change, $last_id;
  if ($last_change && $last_id) {
    variable_set('search_file_attachments_cron_last_change', $last_change);
    variable_set('search_file_attachments_cron_last_id', $last_id);
  }
}

/**
 * Load the files to be indexed.
 *
 * @param int $last_change
 * @param int $last_id
 * @return array
 *   The array with the file objects.
 */
function search_file_attachments_get_files($last_change = 0, $last_id = 0) {
  $limit = (int) variable_get('search_cron_limit', 10);
  $fids = array();

  $query = db_select('file_managed', 'f');
  $query->fields('f', array('fid'))
        ->condition('f.status', 1)
        ->condition(db_or()
          ->condition('f.fid', $last_id, '>')
          ->condition('f.timestamp', $last_change), '>')
        ->orderBy('f.timestamp')
        ->range(0, $limit);
  $results = $query->execute();

  foreach ($results as $record) {
    $fids[] = $record->fid;
  }

  $files = !empty($fids) ? file_load_multiple($fids) : array();
  return $files;
}

/**
 * Extract the content of the given file.
 *
 * @param $file
 *   The file object where the content should be extracted.
 * @return string
 *   The extracted file content.
 */
function search_file_attachments_get_file_content($file) {
  if ($file->filemime == 'text/plain' || $file->filemime == 'text/x-diff') {
    $content = search_file_attachments_extract_simple($file);
  }
  elseif (in_array($file->filemime, array('image/jpeg', 'image/jpg', 'image/tiff'))) {
    $content = search_file_attachments_extract_exif($file);
  }
  else {
    $content = search_file_attachments_extract_tika($file);
  }

  return (string) $content;
}

/**
 * Extract simple text.
 *
 * @param $file
 *   The file object.
 * @return mixed|string
 *   The extracted content.
 */
function search_file_attachments_extract_simple($file) {
  $content = file_get_contents(file_create_url($file->uri));
  $content = iconv("UTF-8", "UTF-8//IGNORE", $content);
  $content = filter_xss(str_replace(array('<', '>'), array(' <', '> '), $content), array());
  $content = htmlspecialchars(html_entity_decode($content, ENT_NOQUOTES, 'UTF-8'), ENT_NOQUOTES, 'UTF-8');
  $content = trim($content);
  return $content;
}

/**
 * Extract IPTC metadata from image.
 *
 * @param $file
 *   The file object.
 * @return string
 *   The extracted IPTC content.
 */
function search_file_attachments_extract_exif($file) {
  $content = '';
  $size = getimagesize(file_create_url($file->uri), $info);
  if (isset($info['APP13'])) {
    $iptc_raw = iptcparse($info['APP13']);
    if (empty($iptc_raw)) {
      return $content;
    }

    $tagmarker = search_file_attachments_exif_tagmarker();

    $iptc = array();
    foreach ($iptc_raw as $key => $value) {
      // Add only values from the defined iptc fields.
      if (array_key_exists($key, $tagmarker)) {
        $iptc_field_value = array();
        foreach ($value as $innerkey => $innervalue) {
          $innervalue = filter_xss(str_replace(array('<', '>'), array(' <', '> '), $innervalue), array());
          $innervalue = trim($innervalue);
          if (!empty($innervalue)) {
            $iptc_field_value[] = $innervalue;
          }
        }

        if (!empty($iptc_field_value)) {
          $iptc[$tagmarker[$key]] = implode(', ', $iptc_field_value);
        }
      }
    }

    foreach ($iptc as $key => $value) {
      $content .= " <strong>{$key}:</strong> {$value}";
    }
    $content = trim($content);
  }
  return $content;
}

/**
 * Extract file content with Apache Tika.
 *
 * @param $file
 *   The file object.
 * @return string
 *   The extracted content.
 * @throws Exception
 */
function search_file_attachments_extract_tika($file) {
  $filepath = file_create_url($file->uri);
  $tika_path = realpath(variable_get('search_file_attachments_tika_path', ''));
  $tika = realpath($tika_path . '/' . variable_get('search_file_attachments_tika_jar', ''));
  if (!($tika) || !is_file($tika)) {
    throw new Exception(t('Invalid path or filename for tika application jar.'));
  }

  $param = '';

  if ($file->filemime != 'audio/mpeg') {
    $param = ' -Dfile.encoding=UTF8 -cp ' . escapeshellarg($tika_path);
  }

  $cmd = escapeshellcmd('java') . $param . ' -jar ' . escapeshellarg($tika) . ' -t ' . escapeshellarg($filepath);
  if (strpos(ini_get('extension_dir'), 'MAMP/')) {
    $cmd = 'export DYLD_LIBRARY_PATH=""; ' . $cmd;
  }

  return shell_exec($cmd);
}

/*
 * Defines the IPTC fields to be used for the search index.
 */
function search_file_attachments_exif_tagmarker() {
  $tagmarker = array(
    '2#005' => t('Object Name'),
    '2#015' => t('Category'),
    '2#020' => t('Supplementals'),
    '2#025' => t('Keywords'),
    '2#040' => t('Special Instructions'),
    '2#080' => t('By Line'),
    '2#085' => t('By Line Title'),
    '2#090' => t('City'),
    '2#092' => t('Sublocation'),
    '2#095' => t('Province State'),
    '2#100' => t('Country Code'),
    '2#101' => t('Country Name'),
    '2#105' => t('Headline'),
    '2#110' => t('Credits'),
    '2#115' => t('Source'),
    '2#116' => t('Copyright'),
    '2#118' => t('Contact'),
    '2#120' => t('Caption'),
    '2#122' => t('Caption Writer'),
  );

  // Allow other modules to alter the editor settings for this format.
  drupal_alter('search_file_attachments_exif_tagmarker', $tagmarker);

  return $tagmarker;
}
