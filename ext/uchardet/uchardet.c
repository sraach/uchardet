#include <ruby.h>
#include <unicode/ucsdet.h>

#ifndef RSTRING_PTR
#	define RSTRING_PTR(str) RSTRING(str)->ptr
#	define RSTRING_LEN(str) RSTRING(str)->len
#endif

static VALUE cUChardetError;
static VALUE cUCharsetDetector;

static void
assure(UErrorCode status)
{
	if (U_FAILURE(status)) {
	    VALUE ex = rb_exc_new2(cUChardetError, u_errorName(status));
	    rb_iv_set(ex, "@errno", INT2FIX(status));
	    rb_exc_raise(ex);
	}
}

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
	assure(status);

	return Data_Wrap_Struct(klass, NULL, UCharsetDetector_free, detector);
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

static VALUE
UCharsetDetector_get_text(VALUE self)
{
    return rb_iv_get(self, "@text");
}

static VALUE
UCharsetDetector_set_text(VALUE self, VALUE text)
{
    return rb_iv_set(self, "@text", text);
	return text;
}

static VALUE
UCharsetDetector_get_declared_encoding(VALUE self)
{
    return rb_iv_get(self, "@declared_encoding");
}

static VALUE
UCharsetDetector_set_declared_encoding(VALUE self, VALUE declared_encoding)
{
    return rb_iv_set(self, "@declared_encoding", declared_encoding);
	return declared_encoding;
}

static VALUE
set_text(VALUE self, VALUE text)
{
	if (!NIL_P(text)) {
		text = StringValue(text);
		
		UErrorCode status = U_ZERO_ERROR;
		UCharsetDetector *detector;
		Data_Get_Struct(self, UCharsetDetector, detector);
		
		ucsdet_setText(detector, StringValuePtr(text), RSTRING_LEN(text), &status);
		assure(status);
	}
	return UCharsetDetector_set_text(self, text);
}

static VALUE
set_declared_encoding(VALUE self, VALUE declared_encoding)
{
	if (!NIL_P(declared_encoding)){
		declared_encoding = StringValue(declared_encoding);
		
		UErrorCode status = U_ZERO_ERROR;
		UCharsetDetector *detector;
		Data_Get_Struct(self, UCharsetDetector, detector);

		ucsdet_setDeclaredEncoding(detector, StringValuePtr(declared_encoding), RSTRING_LEN(declared_encoding), &status);
		assure(status);
	}
	return UCharsetDetector_set_declared_encoding(self, declared_encoding);
}

static VALUE
UCharsetDetector_initialize(int argc, VALUE *argv, VALUE self)
{
	VALUE text;
	VALUE declared_encoding;
	
	rb_scan_args(argc, argv, "02", &text, &declared_encoding);
	set_text(self, text);
	set_declared_encoding(self, declared_encoding);
	
	return self;
}

static VALUE
UCharsetDetector_detect(int argc, VALUE *argv, VALUE self)
{
	VALUE text;
	VALUE declared_encoding;
	
	rb_scan_args(argc, argv, "02", &text, &declared_encoding);
	set_text(self, text);
	set_declared_encoding(self, declared_encoding);
	
	UErrorCode status = U_ZERO_ERROR;
	UCharsetDetector *detector;
	Data_Get_Struct(self, UCharsetDetector, detector);
	
	const UCharsetMatch *match = ucsdet_detect(detector, &status);
	assure(status);
		
	const char *encoding_name = ucsdet_getName(match, &status);
	assure(status);

	int32_t encoding_confidence = ucsdet_getConfidence(match, &status);
	assure(status);
		
	const char *encoding_language = ucsdet_getLanguage(match, &status);
	assure(status);
		
	VALUE hash = rb_hash_new();
	rb_hash_aset(hash, ID2SYM(rb_intern("encoding")), rb_str_new2(encoding_name));
	rb_hash_aset(hash, ID2SYM(rb_intern("confidence")), INT2NUM(encoding_confidence));
	rb_hash_aset(hash, ID2SYM(rb_intern("language")), rb_str_new2(encoding_language));
	
	return hash;
}

static VALUE
UCharsetDetector_detect_all(int argc, VALUE *argv, VALUE self)
{
	VALUE text;
	VALUE declared_encoding;
	
	rb_scan_args(argc, argv, "02", &text, &declared_encoding);
	set_text(self, text);
	set_declared_encoding(self, declared_encoding);
	
	UCharsetDetector *detector;
	Data_Get_Struct(self, UCharsetDetector, detector);
	UErrorCode status = U_ZERO_ERROR;
	int32_t matches_found;
	
	const UCharsetMatch **matches = ucsdet_detectAll(detector, &matches_found, &status);
	assure(status);
	
	VALUE ary = rb_ary_new();
	int i = 0;
	
	for (i = 0; i < matches_found; i++) {
		const char *encoding_name = ucsdet_getName(matches[i], &status);
		assure(status);

		int32_t encoding_confidence = ucsdet_getConfidence(matches[i], &status);
		assure(status);
		
		const char *encoding_language = ucsdet_getLanguage(matches[i], &status);
		assure(status);
		
		VALUE hash = rb_hash_new();
		rb_hash_aset(hash, ID2SYM(rb_intern("encoding")), rb_str_new2(encoding_name));
		rb_hash_aset(hash, ID2SYM(rb_intern("confidence")), INT2NUM(encoding_confidence));
		rb_hash_aset(hash, ID2SYM(rb_intern("language")), rb_str_new2(encoding_language));
		
		rb_ary_push(ary, hash);
	}
	
	return ary;
}

static VALUE
UCharsetDetector_get_detectable_charsets(VALUE self)
{
	UCharsetDetector *detector;
	Data_Get_Struct(self, UCharsetDetector, detector);
	UErrorCode status = U_ZERO_ERROR;
	
	UEnumeration *charsets = ucsdet_getAllDetectableCharsets(detector, &status);
	assure(status);
	
	VALUE ary = rb_ary_new();
	int32_t result_length;
	const char *charset_name;
	
	while (charset_name = uenum_next(charsets, &result_length, &status)) {
		assure(status);
		rb_ary_push(ary, rb_str_new2(charset_name));
	}
	uenum_close(charsets);
	
	return ary;
}

void
Init_uchardet()
{
	VALUE mICU = rb_define_module("ICU");
	
	cUChardetError = rb_define_class_under(mICU, "Error", rb_eStandardError);
	
	cUCharsetDetector = rb_define_class_under(mICU, "UCharsetDetector", rb_cObject);
	rb_define_alloc_func(cUCharsetDetector, UCharsetDetector_alloc);
	rb_define_method(cUCharsetDetector, "initialize", UCharsetDetector_initialize, -1);
	rb_define_method(cUCharsetDetector, "input_filtered?", UCharsetDetector_get_input_filtered, 0);
	rb_define_method(cUCharsetDetector, "input_filtered=", UCharsetDetector_set_input_filtered, 1);
	rb_define_method(cUCharsetDetector, "text", UCharsetDetector_get_text, 0);
	rb_define_method(cUCharsetDetector, "text=", UCharsetDetector_set_text, 1);
	rb_define_method(cUCharsetDetector, "declared_encoding", UCharsetDetector_get_declared_encoding, 0);
	rb_define_method(cUCharsetDetector, "declared_encoding=", UCharsetDetector_set_declared_encoding, 1);
	rb_define_method(cUCharsetDetector, "detect", UCharsetDetector_detect, -1);
	rb_define_method(cUCharsetDetector, "detect_all", UCharsetDetector_detect_all, -1);
	rb_define_method(cUCharsetDetector, "detectable_charsets", UCharsetDetector_get_detectable_charsets, 0);
}
