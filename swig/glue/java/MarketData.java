/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.0
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.opentransactions.otapi;

public class MarketData extends Displayable {
  private long swigCPtr;

  protected MarketData(long cPtr, boolean cMemoryOwn) {
    super(otapiJNI.MarketData_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(MarketData obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        otapiJNI.delete_MarketData(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }
/*@SWIG:swig\otapi\OTAPI.i,181,OT_CAN_BE_CONTAINED_BY@*/
	// Ensure that the GC doesn't collect any OT_CONTAINER instance set from Java
	private MarketList containerRefMarketList;
	// ----------------	
	protected void addReference(MarketList theContainer) {  // This is Java code
		containerRefMarketList = theContainer;
	}
	// ----------------
/*@SWIG@*/
	// ------------------------
  public void setGui_label(String value) {
    otapiJNI.MarketData_gui_label_set(swigCPtr, this, value);
  }

  public String getGui_label() {
    return otapiJNI.MarketData_gui_label_get(swigCPtr, this);
  }

  public void setServer_id(String value) {
    otapiJNI.MarketData_server_id_set(swigCPtr, this, value);
  }

  public String getServer_id() {
    return otapiJNI.MarketData_server_id_get(swigCPtr, this);
  }

  public void setMarket_id(String value) {
    otapiJNI.MarketData_market_id_set(swigCPtr, this, value);
  }

  public String getMarket_id() {
    return otapiJNI.MarketData_market_id_get(swigCPtr, this);
  }

  public void setAsset_type_id(String value) {
    otapiJNI.MarketData_asset_type_id_set(swigCPtr, this, value);
  }

  public String getAsset_type_id() {
    return otapiJNI.MarketData_asset_type_id_get(swigCPtr, this);
  }

  public void setCurrency_type_id(String value) {
    otapiJNI.MarketData_currency_type_id_set(swigCPtr, this, value);
  }

  public String getCurrency_type_id() {
    return otapiJNI.MarketData_currency_type_id_get(swigCPtr, this);
  }

  public void setScale(String value) {
    otapiJNI.MarketData_scale_set(swigCPtr, this, value);
  }

  public String getScale() {
    return otapiJNI.MarketData_scale_get(swigCPtr, this);
  }

  public void setTotal_assets(String value) {
    otapiJNI.MarketData_total_assets_set(swigCPtr, this, value);
  }

  public String getTotal_assets() {
    return otapiJNI.MarketData_total_assets_get(swigCPtr, this);
  }

  public void setNumber_bids(String value) {
    otapiJNI.MarketData_number_bids_set(swigCPtr, this, value);
  }

  public String getNumber_bids() {
    return otapiJNI.MarketData_number_bids_get(swigCPtr, this);
  }

  public void setNumber_asks(String value) {
    otapiJNI.MarketData_number_asks_set(swigCPtr, this, value);
  }

  public String getNumber_asks() {
    return otapiJNI.MarketData_number_asks_get(swigCPtr, this);
  }

  public void setLast_sale_price(String value) {
    otapiJNI.MarketData_last_sale_price_set(swigCPtr, this, value);
  }

  public String getLast_sale_price() {
    return otapiJNI.MarketData_last_sale_price_get(swigCPtr, this);
  }

  public void setCurrent_bid(String value) {
    otapiJNI.MarketData_current_bid_set(swigCPtr, this, value);
  }

  public String getCurrent_bid() {
    return otapiJNI.MarketData_current_bid_get(swigCPtr, this);
  }

  public void setCurrent_ask(String value) {
    otapiJNI.MarketData_current_ask_set(swigCPtr, this, value);
  }

  public String getCurrent_ask() {
    return otapiJNI.MarketData_current_ask_get(swigCPtr, this);
  }

  public void setVolume_trades(String value) {
    otapiJNI.MarketData_volume_trades_set(swigCPtr, this, value);
  }

  public String getVolume_trades() {
    return otapiJNI.MarketData_volume_trades_get(swigCPtr, this);
  }

  public void setVolume_assets(String value) {
    otapiJNI.MarketData_volume_assets_set(swigCPtr, this, value);
  }

  public String getVolume_assets() {
    return otapiJNI.MarketData_volume_assets_get(swigCPtr, this);
  }

  public void setVolume_currency(String value) {
    otapiJNI.MarketData_volume_currency_set(swigCPtr, this, value);
  }

  public String getVolume_currency() {
    return otapiJNI.MarketData_volume_currency_get(swigCPtr, this);
  }

  public void setRecent_highest_bid(String value) {
    otapiJNI.MarketData_recent_highest_bid_set(swigCPtr, this, value);
  }

  public String getRecent_highest_bid() {
    return otapiJNI.MarketData_recent_highest_bid_get(swigCPtr, this);
  }

  public void setRecent_lowest_ask(String value) {
    otapiJNI.MarketData_recent_lowest_ask_set(swigCPtr, this, value);
  }

  public String getRecent_lowest_ask() {
    return otapiJNI.MarketData_recent_lowest_ask_get(swigCPtr, this);
  }

  public void setLast_sale_date(String value) {
    otapiJNI.MarketData_last_sale_date_set(swigCPtr, this, value);
  }

  public String getLast_sale_date() {
    return otapiJNI.MarketData_last_sale_date_get(swigCPtr, this);
  }

  public static MarketData ot_dynamic_cast(Storable pObject) {
    long cPtr = otapiJNI.MarketData_ot_dynamic_cast(Storable.getCPtr(pObject), pObject);
    return (cPtr == 0) ? null : new MarketData(cPtr, false);
  }

}
