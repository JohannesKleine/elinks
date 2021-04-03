/* The SpiderMonkey localstorage object implementation. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elinks.h"

#include "ecmascript/spidermonkey/util.h"

#include "bfu/dialog.h"
#include "cache/cache.h"
#include "config/home.h"
#include "cookies/cookies.h"
#include "dialogs/menu.h"
#include "dialogs/status.h"
#include "document/html/frames.h"
#include "document/document.h"
#include "document/forms.h"
#include "document/view.h"
#include "ecmascript/ecmascript.h"
#include "ecmascript/spidermonkey/form.h"
#include "ecmascript/spidermonkey/location.h"
#include "ecmascript/spidermonkey/localstorage.h"
#include "ecmascript/spidermonkey/localstorage-db.h"
#include "ecmascript/spidermonkey/document.h"
#include "ecmascript/spidermonkey/window.h"
#include "intl/gettext/libintl.h"
#include "main/select.h"
#include "osdep/newwin.h"
#include "osdep/sysname.h"
#include "protocol/http/http.h"
#include "protocol/uri.h"
#include "session/history.h"
#include "session/location.h"
#include "session/session.h"
#include "session/task.h"
#include "terminal/tab.h"
#include "terminal/terminal.h"
#include "util/conv.h"
#include "util/memory.h"
#include "util/string.h"
#include "viewer/text/draw.h"
#include "viewer/text/form.h"
#include "viewer/text/link.h"
#include "viewer/text/vs.h"

#include <time.h>
#include "document/renderer.h"
#include "document/refresh.h"
#include "terminal/screen.h"

/* IMPLEMENTS READ FROM STORAGE USING SQLITE DATABASE */
static unsigned char *
readFromStorage(unsigned char *key)
{

	char * val;

	if (local_storage_ready==0)
	{
		db_prepare_structure(local_storage_filename);
		local_storage_ready=1;
	}

	val = db_query_by_key(local_storage_filename, key);

	//DBG("Read: %s %s %s",local_storage_filename, key, val);

	return (val);
}

/* IMPLEMENTS SAVE TO STORAGE USING SQLITE DATABASE */
static void
saveToStorage(unsigned char *key, unsigned char *val)
{

	if (local_storage_ready==0) {
		db_prepare_structure(local_storage_filename);
		local_storage_ready=1;
	}

	int rows_affected=0;

	rows_affected=db_update_set(local_storage_filename, key, val);

	if (rows_affected==0) {
		rows_affected=db_insert_into(local_storage_filename, key, val);
	}

	// DBG(log, "UPD ROWS: %d KEY: %s VAL: %s",rows_affected,key,val);

}

static bool localstorage_get_property(JSContext *ctx, JS::HandleObject hobj, JS::HandleId hid, JS::MutableHandleValue hvp);

JSClassOps localstorage_ops = {
	JS_PropertyStub, nullptr,
	localstorage_get_property, JS_StrictPropertyStub,
	nullptr, nullptr, nullptr
};

/* Each @localstorage_class object must have a @window_class parent.  */
const JSClass localstorage_class = {
	"localStorage",
	JSCLASS_HAS_PRIVATE,
	&localstorage_ops
};

const JSPropertySpec localstorage_props[] = {
	{ NULL }
};

///* @localstorage_class.getProperty */
static bool
localstorage_get_property(JSContext *ctx, JS::HandleObject hobj, JS::HandleId hid, JS::MutableHandleValue hvp)
{
	JSObject *parent_win;	/* instance of @window_class */

	return(true);
}

static bool localstorage_setitem(JSContext *ctx, unsigned int argc, JS::Value *vp);
static bool localstorage_getitem(JSContext *ctx, unsigned int argc, JS::Value *vp);

const spidermonkeyFunctionSpec localstorage_funcs[] = {
	{ "setItem",		localstorage_setitem,	2 },
	{ "getItem",		localstorage_getitem,	1 },
	{ NULL }
};

static bool
localstorage_getitem(JSContext *ctx, unsigned int argc, JS::Value *vp)
{
	//jsval val;
	JSCompartment *comp = js::GetContextCompartment(ctx);

	if (!comp)
	{
		return false;
	}

	struct ecmascript_interpreter *interpreter = JS_GetCompartmentPrivate(comp);
	JS::CallArgs args = CallArgsFromVp(argc, vp);
        unsigned char *key = JS_EncodeString(ctx, args[0].toString());
	//DBG("localstorage get by key: %s\n", args);

	if (argc != 1)
	{
	       args.rval().setBoolean(false);
	       return(true);
	}

	unsigned char *val;

        val = readFromStorage(key);

	//DBG("%s %s\n", key, val);

	args.rval().setString(JS_NewStringCopyZ(ctx, val));

	mem_free(val);

	return(true);

}

/* @localstorage_funcs{"setItem"} */
static bool
localstorage_setitem(JSContext *ctx, unsigned int argc, JS::Value *vp)
{
	struct string key;
	struct string val;

	init_string(&key);
	init_string(&val);

	JSCompartment *comp = js::GetContextCompartment(ctx);

	if (!comp)
	{
		return false;
	}
	struct ecmascript_interpreter *interpreter = JS_GetCompartmentPrivate(comp);
	JS::CallArgs args = CallArgsFromVp(argc, vp);

        if (argc != 2)
	{
	       args.rval().setBoolean(false);
	       return(true);
        }

	jshandle_value_to_char_string(&key,ctx,&args[0]);
	jshandle_value_to_char_string(&val,ctx,&args[1]);

	saveToStorage(key.source,val.source);

	//DBG("%s %s\n", key, val);


#ifdef CONFIG_LEDS
	set_led_value(interpreter->vs->doc_view->session->status.ecmascript_led, 'J');
#endif
	args.rval().setBoolean(true);

	done_string(&key);
	done_string(&val);

	return(true);
}
