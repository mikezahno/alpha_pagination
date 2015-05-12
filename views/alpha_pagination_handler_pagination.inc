<?php

/**
 * @file
 * Definition of views_handler_area_result.
 */

/**
 * Views area handler to display some configurable result summary.
 *
 * @ingroup views_area_handlers
 */
class alpha_pagination_handler_pagination extends views_handler_area {

  function option_definition() {
    // $options = parent::option_definition();
    $options['pre_letter_path'] = array(
      'default' => 'by-first-letter',
      'translatable' => FALSE,
    );
    return $options;
  }

  function options_form(&$form, &$form_state) {
    // parent::options_form($form, $form_state);
    $form['pre_letter_path'] = array(
      '#title' => t('Path To Results View'),
      '#type' => 'textfield',
      '#size' => 60,
      '#default_value' => $this->options['pre_letter_path'],
      '#description' => t('This is the path to the view that handles the search by letter. No beginning or ending slashes.'),
    );
  }
  
  function post_execute() {
    $query = ($this->view->build_info['query']);
    $quoted = $query->getArguments();
    $connection = Database::getConnection();
    foreach ($quoted as $key => $val) {
      if (is_array($val)) {
        $quoted[$key] = implode(', ', array_map(array($connection, 'quote'), $val));
      }
      else {
        $quoted[$key] = $connection->quote($val);
      }
    }
    $this->options['query'] = check_plain(strtr($query, $quoted));
  }

  /**
   * Find out the information to render.
   */
  function render($empty = FALSE) {
    $query_parts = explode("\n", $this->options['query']);
    
    foreach ($query_parts as $k => $part) {
      if ($position = strpos($part, "SUBSTRING")) {
        $part = substr($part, 0, $position) . " 1 OR " . substr($part, $position);
        $query_parts[$k] = $part;
      }
    }
      
    $last_line = array_pop($query_parts);
    if (substr($last_line, 0, 5) != "LIMIT") {
      $query_parts[] = $last_line;
    }
    $query = join("\n", $query_parts);
    $query = str_replace("&#039;", '\'', $query);

    $result = db_query($query);
    
    $nids = array();
    while($idata = $result->fetchObject()) {
      $nids[] = $idata->nid;
    }
    
    $alphabet = range('A', 'Z');
    $has_results = array();
    foreach ($alphabet as $letter) {
      $has_results[$letter] = FALSE;
    }
    
    if (count($nids) > 0) {
      $result = db_query('SELECT DISTINCT(SUBSTR(field_tweet_contents_value, 1, 1)) AS letter 
                          FROM {field_data_field_tweet_contents} WHERE entity_id IN ( :nids )', array(':nids' => $nids));
      while($idata = $result->fetchObject()) {
        $has_results[strtoupper($idata->letter)] = TRUE;
      }
    }
        
    $output_array = array();
    foreach ($alphabet as $letter) {
      if ($has_results[$letter] == TRUE) {
        $output_array[] = '<a href="/' . $this->options['pre_letter_path']. '/' . $letter . '">' . $letter . '</a>';
      }
      else {
        $output_array[] = $letter;
      }
    }
    $output = join(' | ', $output_array);        
    return $output;
  }
  
}
