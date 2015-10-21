// WmsLayer.cpp : Implementation of CWmsLayer
#include "stdafx.h"
#include "WmsLayer.h"
#include "RamCache.h"

// *********************************************************************
//		~CWmsLayer
// *********************************************************************
CWmsLayer::~CWmsLayer()
{
	Close();

	::SysFreeString(_key);

	// CustomProjection instance of this provider is assigned to all tiles in RAM cache
	// so pointers will become invalid after closing it. We are solving this by reassigning a pointer 
	// on extracting a tile from cache. But let's keep it clean and nullify invalid pointers as well.
	RamCache* cache = dynamic_cast<RamCache*>(_manager.get_RamCache()->cache);
	if (cache) {
		cache->OnProviderClosed(_provider->Id);
	}

	delete _provider;

	gReferenceCounter.Release(tkInterface::idWmsLayer);
}

// *********************************************************************
//		Load
// *********************************************************************
void CWmsLayer::Load(IMapViewCallback* map, bool isSnapshot /*= false*/, CString key /*= ""*/)
{
	if (!_screenBuffer) 
	{
		ResizeBuffer(map->_GetWidth(), map->_GetHeight());
	}

	_manager.set_MapCallback(map);

	_provider->get_CustomProjection()->UpdateBounds();

	_manager.LoadTiles(_provider, isSnapshot, key);
}

// *********************************************************************
//		ResizeBuffer
// *********************************************************************
void CWmsLayer::ResizeBuffer(int cx, int cy)
{
	if (_screenBuffer)
	{
		delete _screenBuffer;
		_screenBuffer = NULL;
	}

	if (cx > 0 && cy > 0) {
		_screenBuffer = new Gdiplus::Bitmap(cx, cy);
	}
}

// *********************************************************************
//		get_LastErrorCode
// *********************************************************************
STDMETHODIMP CWmsLayer::get_LastErrorCode(long *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	*pVal = _lastErrorCode;
	_lastErrorCode = tkNO_ERROR;
	return S_OK;
}

// *********************************************************************
//		get_ErrorMsg
// *********************************************************************
STDMETHODIMP CWmsLayer::get_ErrorMsg(long ErrorCode, BSTR *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	USES_CONVERSION;
	*pVal = A2BSTR(ErrorMsg(ErrorCode));
	return S_OK;
}

// *********************************************************************
//		get/put_Key
// *********************************************************************
STDMETHODIMP CWmsLayer::get_Key(BSTR *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	USES_CONVERSION;
	*pVal = OLE2BSTR(_key);

	return S_OK;
}

STDMETHODIMP CWmsLayer::put_Key(BSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	USES_CONVERSION;
	::SysFreeString(_key);
	_key = OLE2BSTR(newVal);

	return S_OK;
}

// *********************************************************
//	     Name()
// *********************************************************
STDMETHODIMP CWmsLayer::get_Name(BSTR* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	USES_CONVERSION;
	*pVal = A2BSTR(_provider->Name);

	return S_OK;
}
STDMETHODIMP CWmsLayer::put_Name(BSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	USES_CONVERSION;
	_provider->Name = OLE2A(newVal);

	return S_OK;
}

// *********************************************************
//	     BoundingBox()
// *********************************************************
STDMETHODIMP CWmsLayer::get_BoundingBox(IExtents** pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	*pVal = NULL;

	CustomProjection* projection = _provider->get_CustomProjection();
	if (projection) 
	{
		double xMin, xMax, yMin, yMax;
		projection->get_Bounds(xMin, xMax, yMin, yMax);

		IExtents* box = NULL;
		ComHelper::CreateExtents(&box);
		
		box->SetBounds(xMin, yMin, 0.0, xMax, yMax, 0.0);

		*pVal = box;
	}

	return S_OK;
}

STDMETHODIMP CWmsLayer::put_BoundingBox(IExtents* newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!newVal) 
	{
		ErrorMsg(tkUNEXPECTED_NULL_PARAMETER);
		return S_OK;
	}

	CustomProjection* projection = _provider->get_CustomProjection();
	if (projection)
	{
		double xMin, yMin, zMin, xMax, yMax, zMax;
		newVal->GetBounds(&xMin, &yMin, &zMin, &xMax, &yMax, &zMax);
		projection->put_Bounds(xMin, xMax, yMin, yMax);
	}

	return S_OK;
}

// *********************************************************
//	     CrsEpsg()
// *********************************************************
STDMETHODIMP CWmsLayer::get_Epsg(LONG* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	*pVal = _provider->get_CustomProjection()->get_Epsg();

	return S_OK;
}

STDMETHODIMP CWmsLayer::put_Epsg(LONG newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (newVal < 0) return S_OK;

	_provider->get_CustomProjection()->put_Epsg(newVal);

	return S_OK;
}

// *********************************************************
//	     LayersCsv()
// *********************************************************
STDMETHODIMP CWmsLayer::get_Layers(BSTR* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	USES_CONVERSION;
	*pVal = A2BSTR(_provider->get_Layers());

	return S_OK;
}
STDMETHODIMP CWmsLayer::put_Layers(BSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	USES_CONVERSION;
	_provider->set_Layers(OLE2A(newVal));

	return S_OK;
}

// *********************************************************
//	     BaseUrl()
// *********************************************************
STDMETHODIMP CWmsLayer::get_BaseUrl(BSTR* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString s = _provider->get_UrlFormat();

	USES_CONVERSION;
	*pVal = A2BSTR(s);

	return S_OK;
}
STDMETHODIMP CWmsLayer::put_BaseUrl(BSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	USES_CONVERSION;
	CString s = OLE2A(newVal);

	_provider->put_UrlFormat(s);

	return S_OK;
}

// *********************************************************
//	     Id()
// *********************************************************
STDMETHODIMP CWmsLayer::get_Id(LONG* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	*pVal = _provider->Id;

	return S_OK;
}

STDMETHODIMP CWmsLayer::put_Id(LONG newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	_provider->Id = newVal;

	return S_OK;
}

// *********************************************************
//	     Format()
// *********************************************************
STDMETHODIMP CWmsLayer::get_Format(BSTR* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	USES_CONVERSION;
	*pVal = A2BSTR(_provider->get_Format());

	return S_OK;
}

STDMETHODIMP CWmsLayer::put_Format(BSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	USES_CONVERSION;
	_provider->set_Format(OLE2A(newVal));

	return S_OK;
}

// *********************************************************
//	     IsEmpty()
// *********************************************************
STDMETHODIMP CWmsLayer::get_IsEmpty(VARIANT_BOOL* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (_provider->get_UrlFormat().GetLength() == 0 ||
		_provider->get_Format().GetLength() == 0 ||
		_provider->get_Layers().GetLength() == 0 ||
		_provider->get_CustomProjection()->get_Epsg() <= 0)
	{
		*pVal = VARIANT_TRUE;
	}

	return S_OK;
}

// *********************************************************
//	     MapExtents()
// *********************************************************
STDMETHODIMP CWmsLayer::get_MapExtents(IExtents** pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	BaseProjection* p = _provider->get_Projection();
	if (p) 
	{
		IExtents* ext = NULL;
		ComHelper::CreateExtents(&ext);
		ext->SetBounds(p->MapBounds.left, p->MapBounds.bottom, 0.0, p->MapBounds.right, p->MapBounds.top, 0.0);
		*pVal = ext;
	}

	return S_OK;
}

// *********************************************************
//	     Close()
// *********************************************************
STDMETHODIMP CWmsLayer::Close()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (_screenBuffer) {
		delete _screenBuffer;
		_screenBuffer = NULL;
	}

	// TODO: implement

	return S_OK;
}

// *********************************************************
//	     Serialize()
// *********************************************************
STDMETHODIMP CWmsLayer::Serialize(BSTR* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CPLXMLNode* psTree = this->SerializeCore("WmsLayerClass");
	Utility::SerializeAndDestroyXmlTree(psTree, retVal);

	return S_OK;
}

// *********************************************************
//	     Deserialize()
// *********************************************************
STDMETHODIMP CWmsLayer::Deserialize(BSTR state, VARIANT_BOOL* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	*retVal = VARIANT_FALSE;

	USES_CONVERSION;
	CString s = OLE2CA(state);

	CPLXMLNode* node = CPLParseXMLString(s.GetString());
	if (node)
	{
		CPLXMLNode* nodeWmsLayer = CPLGetXMLNode(node, "=WmsLayerClass");
		if (nodeWmsLayer)
		{
			*retVal = DeserializeCore(nodeWmsLayer) ? VARIANT_TRUE : VARIANT_FALSE;
		}

		CPLDestroyXMLNode(node);
	}

	return S_OK;
}

// *********************************************************
//	     get_GeoProjection()
// *********************************************************
STDMETHODIMP CWmsLayer::get_GeoProjection(IGeoProjection** pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CustomProjection* projection = _provider->get_CustomProjection();
	if (projection)
	{
		IGeoProjection* gp = projection->get_ServerProjection();
		if (gp) {
			gp->AddRef();
			*pVal = gp;
		}
	}

	return S_OK;
}

// *********************************************************
//	     Opacity()
// *********************************************************
STDMETHODIMP CWmsLayer::get_Opacity(BYTE* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	*pVal = _manager.get_Alpha();

	return S_OK;
}

STDMETHODIMP CWmsLayer::put_Opacity(BYTE newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	_manager.MarkUndrawn();
	_manager.set_Alpha(newVal);

	return S_OK;
}

// ********************************************************
//     SerializeCore()
// ********************************************************
CPLXMLNode* CWmsLayer::SerializeCore(CString ElementName)
{
	USES_CONVERSION;
	CPLXMLNode* psTree = CPLCreateXMLNode(NULL, CXT_Element, ElementName);
	
	Utility::CPLCreateXMLAttributeAndValue(psTree, "Id", CPLString().Printf("%d", _provider->Id));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "Name", _provider->Name);
	Utility::CPLCreateXMLAttributeAndValue(psTree, "Layers", _provider->get_Layers());
	Utility::CPLCreateXMLAttributeAndValue(psTree, "Url", _provider->get_UrlFormat());
	Utility::CPLCreateXMLAttributeAndValue(psTree, "Format", _provider->get_Format());
	Utility::CPLCreateXMLAttributeAndValue(psTree, "Opactiy", CPLString().Printf("%d", _manager.get_Alpha()));

	long epsg;
	get_Epsg(&epsg);

	Utility::CPLCreateXMLAttributeAndValue(psTree, "Epsg", CPLString().Printf("%d", epsg));

	CComPtr<IExtents> box = NULL;
	get_BoundingBox(&box);

	double xMin, yMin, zMin, xMax, yMax, zMax;
	box->GetBounds(&xMin, &yMin, &zMin, &xMax, &yMax, &zMax);

	Utility::CPLCreateXMLAttributeAndValue(psTree, "xMin", CPLString().Printf("%f", xMin));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "xMax", CPLString().Printf("%f", xMax));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "yMin", CPLString().Printf("%f", yMin));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "yMax", CPLString().Printf("%f", yMax));

	return psTree;
}

// ********************************************************
//     DeserializeCore()
// ********************************************************
bool CWmsLayer::DeserializeCore(CPLXMLNode* node)
{
	if (!node)
		return false;

	CString s = CPLGetXMLValue(node, "Id", NULL);
	if (s != "") put_Id(atoi(s));

	s = CPLGetXMLValue(node, "Name", NULL);
	if (s != "") _provider->Name = s;

	s = CPLGetXMLValue(node, "Layers", NULL);
	if (s != "") _provider->set_Layers(s);

	s = CPLGetXMLValue(node, "Url", NULL);
	if (s != "") _provider->put_UrlFormat(s);

	s = CPLGetXMLValue(node, "Format", NULL);
	if (s != "") _provider->set_Format(s);

	s = CPLGetXMLValue(node, "Opacity", NULL);
	if (s != "") _manager.set_Alpha(atoi(s));

	s = CPLGetXMLValue(node, "Epsg", NULL);
	if (s != "") put_Epsg(atoi(s));
	
	double xMin, xMax, yMin, yMax, zMin = 0.0, zMax = 0.0;
	s = CPLGetXMLValue(node, "xMin", NULL);
	if (s != "") xMin = Utility::atof_custom(s);

	s = CPLGetXMLValue(node, "xMax", NULL);
	if (s != "") xMax = Utility::atof_custom(s);

	s = CPLGetXMLValue(node, "yMin", NULL);
	if (s != "") yMin = Utility::atof_custom(s);

	s = CPLGetXMLValue(node, "yMax", NULL);
	if (s != "") yMax = Utility::atof_custom(s);

	CComPtr<IExtents> box = NULL;
	ComHelper::CreateExtents(&box);
	box->SetBounds(xMin, yMin, zMin, xMax, yMax, zMax);
	put_BoundingBox(box);

	return true;
}