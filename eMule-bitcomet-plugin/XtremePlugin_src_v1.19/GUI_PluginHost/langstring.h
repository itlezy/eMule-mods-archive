#ifndef LANG_STRING
#define LNAG_STRING


class CLangString
{
public:
	enum LangIndex
	{
		LANG_EN, LANG_ZH,
	};

	enum
	{
		LANG_NUM = 2,
	};

	enum StringIndex
	{
		S_HELP,
		//S_ADD_PORT_MAP_SUCCEED,
		//S_ADD_PORT_MAP_FAIL,
		//S_DELETE_PORT_MAP_SUCCEED,
		//S_DELETE_PORT_MAP_FAIL,
		//S_ADD_FIREWALL_PORT_SUCCEED,
		//S_ADD_FIREWALL_PORT_FAIL,
		//S_DELETE_FIREWALL_PORT_SUCCEED,
		//S_DELETE_FIREWALL_PORT_FAIL,
	};

	static TCHAR* get_string(StringIndex index) {return m_string_list[index][m_lang_index];};
	static void set_lang(LangIndex index) {if(index>=0 && index<LANG_NUM) m_lang_index = index;};
	static LangIndex get_lang() {return m_lang_index;};

protected:
	static TCHAR* m_string_list[][LANG_NUM];
	static LangIndex m_lang_index;
	
};

#define STRING(x) CLangString::get_string(CLangString::x)

#endif //LNAG_STRING