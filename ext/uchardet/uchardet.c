#include <ruby.h>
#include <unicode/ucsdet.h>

#ifndef RSTRING_PTR
#	define RSTRING_PTR(str) RSTRING(str)->ptr
#endif
#ifndef RSTRING_LEN
#	define RSTRING_LEN(str) RSTRING(str)->len
#endif

static VALUE cUChardetError;
static VALUE cUCharsetDetector;

static void uchardet_raise(UErrorCode status);

static void
UCharsetDetector_free(void *detector)
{
	ucsdet_close(detector);
}

static VALUE
UCharsetDetector_alloc(VALUE klass)
{
	UErrorCode status = U_ZERO_ERROR;
	UCharsetDetector* detector = ucsdet_open(&status);
	
	if (U_FAILURE(status))
		uchardet_raise(status);

	return Data_Wrap_Struct(klass, NULL, UCharsetDetector_free, detector);
}

static void
_set_text(VALUE self, VALUE text)
{
	if (!NIL_P(text)) {
		text = StringValue(text);
		
		UErrorCode status = U_ZERO_ERROR;
		UCharsetDetector *detector;
		Data_Get_Struct(self, UCharsetDetector, detector);
		
		ucsdet_setText(detector, StringValuePtr(text), RSTRING_LEN(text), &status);
		
		if (U_FAILURE(status))
			uchardet_raise(status);
	}
}

static void
_set_declared_encoding(VALUE self, VALUE encoding)
{
	if (!NIL_P(encoding)){
		encoding = StringValue(encoding);
		
		UErrorCode status = U_ZERO_ERROR;
		UCharsetDetector *detector;
		Data_Get_Struct(self, UCharsetDetector, detector);
		
		ucsdet_setDeclaredEncoding(detector, StringValuePtr(encoding), RSTRING_LEN(encoding), &status);
		
		if (U_FAILURE(status))
			uchardet_raise(status);
	}
}

static VALUE
UCharsetDetector_initialize(int argc, VALUE *argv, VALUE self)
{
	VALUE text;
	
	rb_scan_args(argc, argv, "01", &text);
	_set_text(self, text);
	
	return self;
}

static VALUE
UCharsetDetector_detect(int argc, VALUE *argv, VALUE self)
{
	VALUE text;
	VALUE encoding;
	
	rb_scan_args(argc, argv, "02", &text, &encoding);
	_set_text(self, text);
	_set_declared_encoding(self, encoding);
	
	UErrorCode status = U_ZERO_ERROR;
	UCharsetDetector *detector;
	Data_Get_Struct(self, UCharsetDetector, detector);
	
	const UCharsetMatch *match = ucsdet_detect(detector, &status);
	if (U_FAILURE(status))
		uchardet_raise(status);
		
	const char *encoding_name = ucsdet_getName(match, &status);
	if (U_FAILURE(status))
		uchardet_raise(status);

	int32_t encoding_confidence = ucsdet_getConfidence(match, &status);
	if (U_FAILURE(status))
		uchardet_raise(status);
		
	const char *encoding_language = ucsdet_getLanguage(match, &status);
	if (U_FAILURE(status))
		uchardet_raise(status);
		
	VALUE hash = rb_hash_new();
	rb_hash_aset(hash, ID2SYM(rb_intern("name")), rb_str_new2(encoding_name));
	rb_hash_aset(hash, ID2SYM(rb_intern("confidence")), INT2NUM(encoding_confidence));
	rb_hash_aset(hash, ID2SYM(rb_intern("language")), rb_str_new2(encoding_language));
	
	return hash;
}

static VALUE
UCharsetDetector_detect_all(int argc, VALUE *argv, VALUE self)
{
	VALUE text;
	VALUE encoding;
	
	rb_scan_args(argc, argv, "02", &text, &encoding);
	_set_text(self, text);
	_set_declared_encoding(self, encoding);
	
	UCharsetDetector *detector;
	Data_Get_Struct(self, UCharsetDetector, detector);
	UErrorCode status = U_ZERO_ERROR;
	int32_t matches_found;
	
	const UCharsetMatch **matches = ucsdet_detectAll(detector, &matches_found, &status);
	if (U_FAILURE(status))
		uchardet_raise(status);
	
	VALUE ary = rb_ary_new();
	int i = 0;
	
	for (i = 0; i < matches_found; i++) {
		const char *encoding_name = ucsdet_getName(matches[i], &status);
		if (U_FAILURE(status))
			uchardet_raise(status);

		int32_t encoding_confidence = ucsdet_getConfidence(matches[i], &status);
		if (U_FAILURE(status))
			uchardet_raise(status);
		
		const char *encoding_language = ucsdet_getLanguage(matches[i], &status);
		if (U_FAILURE(status))
			uchardet_raise(status);
		
		VALUE hash = rb_hash_new();
		rb_hash_aset(hash, ID2SYM(rb_intern("name")), rb_str_new2(encoding_name));
		rb_hash_aset(hash, ID2SYM(rb_intern("confidence")), INT2NUM(encoding_confidence));
		rb_hash_aset(hash, ID2SYM(rb_intern("language")), rb_str_new2(encoding_language));
		
		rb_ary_push(ary, hash);
	}
	
	return ary;
}

static VALUE
UCharsetDetector_get_all_detectable_charsets(VALUE self)
{
	UCharsetDetector *detector;
	Data_Get_Struct(self, UCharsetDetector, detector);
	UErrorCode status = U_ZERO_ERROR;
	
	UEnumeration *charsets = ucsdet_getAllDetectableCharsets(detector, &status);
	if (U_FAILURE(status))
		uchardet_raise(status);
	
	VALUE ary = rb_ary_new();
	int32_t result_length;
	const char *charset_name;
	
	while (charset_name = uenum_next(charsets, &result_length, &status)) {
		if (U_FAILURE(status)) {
			uenum_close(charsets);
			uchardet_raise(status);
		}
		else
			rb_ary_push(ary, rb_str_new2(charset_name));
	}
	uenum_close(charsets);
	
	return ary;
}

static VALUE
UCharsetDetector_get_input_filtered(VALUE self)
{
	UCharsetDetector *detector;
	Data_Get_Struct(self, UCharsetDetector, detector);
	
	return ucsdet_isInputFilterEnabled(detector) ? Qtrue : Qfalse;
}

static VALUE
UCharsetDetector_set_input_filtered(VALUE self, VALUE flag)
{
	UCharsetDetector *detector;
	Data_Get_Struct(self, UCharsetDetector, detector);
	
	ucsdet_enableInputFilter(detector, RTEST(flag) ? TRUE : FALSE);
	return self;
}

void
Init_uchardet()
{
	VALUE mICU = rb_define_module("ICU");
	
	cUChardetError = rb_define_class_under(mICU, "Error", rb_eStandardError);
	
	cUCharsetDetector = rb_define_class_under(mICU, "UCharsetDetector", rb_cObject);
	rb_define_alloc_func(cUCharsetDetector, UCharsetDetector_alloc);
	rb_define_method(cUCharsetDetector, "initialize", UCharsetDetector_initialize, -1);
	rb_define_method(cUCharsetDetector, "detect", UCharsetDetector_detect, -1);
	rb_define_method(cUCharsetDetector, "detect_all", UCharsetDetector_detect_all, -1);
	rb_define_method(cUCharsetDetector, "detectable_charsets", UCharsetDetector_get_all_detectable_charsets, 0);
	rb_define_method(cUCharsetDetector, "input_filtered?", UCharsetDetector_get_input_filtered, 0);
	rb_define_method(cUCharsetDetector, "input_filtered=", UCharsetDetector_set_input_filtered, 1);
}

static void
uchardet_raise(UErrorCode status)
{
    VALUE ex = rb_exc_new2(cUChardetError, u_errorName(status));
    rb_iv_set(ex, "@errno", INT2FIX(status));
    rb_exc_raise(ex);
}
