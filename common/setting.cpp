#include "stdafx.h"
#include "setting.h"
#include <comutil.h>

_gdimm_setting::_gdimm_setting()
{
	setting_map gdimm_default;
	gdimm_default[L"auto_hinting"] = L"0";
	gdimm_default[L"bold_strength"] = L"0.0";
	gdimm_default[L"freetype_loader"] = L"1";
	gdimm_default[L"hinting"] = L"1";
	gdimm_default[L"lcd_filter"] = L"1";
	gdimm_default[L"light_mode"] = L"0";
	gdimm_default[L"max_height"] = L"72";
	gdimm_default[L"render_mono"] = L"0";
	gdimm_default[L"subpixel_render"] = L"1";

	_setting_branchs.push_back(gdimm_default);
	_branch_names[L"common"] = &_setting_branchs.back();
}

void _gdimm_setting::load_branch(IXMLDOMDocument *xml_doc, WCHAR *xpath)
{
	HRESULT hr;

	IXMLDOMNodeList *branch_list;
	IXMLDOMNode *curr_branch;
	setting_map *branch_setting;

	hr = xml_doc->selectNodes(xpath, &branch_list);
	assert(SUCCEEDED(hr));

	while (true)
	{
		hr = branch_list->nextNode(&curr_branch);
		if (curr_branch == NULL)
			break;

		// if the node has the attribute "name", use it as the branch name
		// otherwise, use the node name as the branch name
		IXMLDOMNamedNodeMap *node_attr;
		hr = curr_branch->get_attributes(&node_attr);
		assert(SUCCEEDED(hr));
		
		IXMLDOMNode *name_node;
		hr = node_attr->getNamedItem(L"name", &name_node);
		assert(SUCCEEDED(hr));

		BSTR branch_name;
		if (name_node == NULL)
			hr = curr_branch->get_nodeName(&branch_name);
		else
		{
			hr = name_node->get_text(&branch_name);
			name_node->Release();
		}
		assert(SUCCEEDED(hr));
		
		// create new setting branch if necessary, otherwise use the existing
		branch_map::const_iterator iter = _branch_names.find(branch_name);
		if (iter == _branch_names.end())
		{
			_setting_branchs.push_back(setting_map());
			branch_setting = &_setting_branchs.back();
			_branch_names[branch_name] = branch_setting;
		}
		else
			branch_setting = iter->second;
		
		// add all child nodes to the setting branch
		IXMLDOMNodeList *setting_list;
		hr = curr_branch->get_childNodes(&setting_list);
		assert(SUCCEEDED(hr));

		IXMLDOMNode *setting_node;
		while (true)
		{
			hr = setting_list->nextNode(&setting_node);
			if (setting_node == NULL)
				break;

			BSTR setting_name;
			BSTR setting_value;

			hr = setting_node->get_nodeName(&setting_name);
			assert(SUCCEEDED(hr));

			hr = setting_node->get_text(&setting_value);
			assert(SUCCEEDED(hr));

			(*branch_setting)[setting_name] = setting_value;

			setting_node->Release();
		}

		curr_branch->Release();
		setting_list->Release();
		node_attr->Release();
	}

	branch_list->Release();
}

void _gdimm_setting::load_exclude(IXMLDOMDocument *xml_doc)
{
	HRESULT hr;

	IXMLDOMNodeList *node_list;
	IXMLDOMNode *curr_node;

	hr = xml_doc->selectNodes(L"/gdipp/exclude/*", &node_list);
	assert(SUCCEEDED(hr));

	while (true)
	{
		hr = node_list->nextNode(&curr_node);
		if (curr_node == NULL)
			break;

		BSTR proc_name;
		hr = curr_node->get_text(&proc_name);
		assert(SUCCEEDED(hr));

		_exclude_names.insert(proc_name);
	}
}

void _gdimm_setting::load_settings(HMODULE h_module)
{
	HRESULT hr;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	IXMLDOMDocument * xml_doc;
	hr = CoCreateInstance(__uuidof(DOMDocument), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&xml_doc));
	assert(SUCCEEDED(hr));

	// get setting file path
	WCHAR setting_path[MAX_PATH];
	get_dir_file_path(setting_path, L"setting.xml", h_module);

	VARIANT var_file_path;
	VariantInit(&var_file_path);
	V_BSTR(&var_file_path) = bstr_t(setting_path);
	V_VT(&var_file_path) = VT_BSTR;

	VARIANT_BOOL var_success;
	hr = xml_doc->load(var_file_path, &var_success);
	assert(SUCCEEDED(hr) && var_success);
	VariantClear(&var_file_path);

	if (h_module != NULL)
	{
		load_branch(xml_doc, L"/gdipp/gdimm/common");
		load_branch(xml_doc, L"/gdipp/gdimm/proc");
		load_branch(xml_doc, L"/gdipp/gdimm/font");
	}

	load_exclude(xml_doc);

	xml_doc->Release();

	CoUninitialize();
}