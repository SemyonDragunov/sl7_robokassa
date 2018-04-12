<?php

/**
 * @author Semyon Dragunov <sam.dragunov@gmail.com>
 * https://github.com/SemyonDragunov
 */

class SL7RobokassaPaymentController extends SL7PaymentController {

  protected $description;

  /**
   * Переопределяем SL7PaymentController::checkup()
   */
  public function checkup() {
    if (!empty($this->payment_type) && !empty($this->description)) {
      return TRUE;
    }

    return FALSE;
  }

  /**
   * Устанавливет описание платежа.
   * Берет за описание название первого товара в платеже.
   */
  public function description($order) {
    $items = sl7_order_get_items($order);
    $this->description = $items[0]['label'];
  }

  private function _query($order) {
    $login = variable_get('sl7_robokassa_login');
    $secret_key_1 = variable_get('sl7_robokassa_secret_key_1');
    $oid = $order->oid;
    $sum = sl7_order_get_sum($order->oid);
    $md5 = md5("$login:$sum:$oid:$secret_key_1");

    $query = array(
      'MrchLogin' => $login,
      'InvId' => $order->oid,
      'Desc' => $this->description,
      'OutSum' => $sum,
      'SignatureValue' => $md5,
      'IncCurrLabel' => variable_get('sl7_robokassa_curr_label', ''),
      'Culture' => variable_get('sl7_robokassa_culture', 'ru'),
    );

    global $user;
    if ($user->mail) $query['Email'] = $user->mail;

    return $query;
  }

  public function external($order, $ajax = FALSE) {
    $order = is_object($order) ? $order : sl7_order_load($order);
    $this->description($order);

    if (!$this->checkup()) exit;

    $url = sl7_robokassa_server_url(variable_get('sl7_robokassa_merchant_server_name'));

    if ($ajax) {
      ctools_include('ajax');
      ctools_add_js('ajax-responder');
      $commands[] = ctools_ajax_command_redirect($url, 0, array('query' => $this->_query($order)));
      print ajax_render($commands);
      exit;
    }
    else {
      drupal_goto($url, array('query' => $this->_query($order)));
    }
  }
}