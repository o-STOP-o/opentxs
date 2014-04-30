/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.0
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.opentransactions.otapi;

public class TradeListNym extends Storable {
  private long swigCPtr;

  protected TradeListNym(long cPtr, boolean cMemoryOwn) {
    super(otapiJNI.TradeListNym_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(TradeListNym obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        otapiJNI.delete_TradeListNym(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }
// ------------------------
	/*@SWIG:swig\otapi\OTAPI.i,135,OT_CONTAINER_TYPE_MEMBERS@*/
	private List elementList = new ArrayList();
/*@SWIG@*/
	/*@SWIG:swig\otapi\OTAPI.i,212,OT_ADD_ELEMENT@*/  // THIS BLOCK CONTAINS JAVA CODE.
private Long removeRefTradeDataNym(Long lIndex) {
	// 
	// loop through the elements in the actual container, in order to find the one
	// at lIndex. Once it is found, then loop through the reference list and remove
	// the corresponding reference for that element.
	//
	TradeDataNym refActualElement = GetTradeDataNym(lIndex);

	if (refActualElement == null)
		return lIndex; // oh well.
	
	// Loop through the reference list and remove the corresponding reference
	// for the specified element.
	//
	for(Integer intIndex = 0; intIndex < elementList.size(); intIndex++)
	{
		Object theObject = elementList.get(intIndex);
		
		if ((theObject == null) || !(theObject instanceof TradeDataNym))
			continue;

		TradeDataNym tempRef = (TradeDataNym)(theObject);
		
		if ((TradeDataNym.getCPtr(tempRef) == TradeDataNym.getCPtr(refActualElement)))
		{
			elementList.remove(tempRef);
			break;
		}
	}
	
	return lIndex;
}

private Long getCPtrAddRefTradeDataNym(TradeDataNym element) {
	// Whenever adding a reference to the list, I remove it first (if already there.)
	// That way we never store more than one reference per actual contained object.
	//
	for(Integer intIndex = 0; intIndex < elementList.size(); intIndex++)
	{
		Object theObject = elementList.get(intIndex);

		if ((theObject == null) || !(theObject instanceof TradeDataNym))
			continue;
		
		TradeDataNym tempRef = (TradeDataNym)(theObject);
		
		if ((TradeDataNym.getCPtr(tempRef) == TradeDataNym.getCPtr(element)))
		{
			elementList.remove(tempRef); // It was already there, so let's remove it before adding (below.)
			break;
		}
	}
	// Now we add it...
	//
	TradeDataNym tempLocalRef = element;
	elementList.add(tempLocalRef);
	return TradeDataNym.getCPtr(element);
}	// Hope I get away with overloading this for every type. Otherwise,
/*@SWIG@*/
  public long GetTradeDataNymCount() {
    return otapiJNI.TradeListNym_GetTradeDataNymCount(swigCPtr, this);
  }

  public TradeDataNym GetTradeDataNym(long nIndex) {
    long cPtr = otapiJNI.TradeListNym_GetTradeDataNym(swigCPtr, this, nIndex);
    return (cPtr == 0) ? null : new TradeDataNym(cPtr, false);
  }

  public boolean RemoveTradeDataNym(long nIndexTradeDataNym) {
    return otapiJNI.TradeListNym_RemoveTradeDataNym(swigCPtr, this, removeRefTradeDataNym(nIndexTradeDataNym));
  }

  public boolean AddTradeDataNym(TradeDataNym disownObject) {
    return otapiJNI.TradeListNym_AddTradeDataNym(swigCPtr, this, TradeDataNym.getCPtr(disownObject), disownObject);
  }

  public static TradeListNym ot_dynamic_cast(Storable pObject) {
    long cPtr = otapiJNI.TradeListNym_ot_dynamic_cast(Storable.getCPtr(pObject), pObject);
    return (cPtr == 0) ? null : new TradeListNym(cPtr, false);
  }

}
