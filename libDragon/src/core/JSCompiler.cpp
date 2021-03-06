#include "JSCompiler.h"

#include <cstring>
#include <cstdio>
#include <cassert>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include "modules/Console.h"

NS_USING_DRAGON
NS_USING_V8
NS_USING_BOOST

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch);

JavaScriptCompiler::JavaScriptCompiler()
{
	isolate_ = Isolate::GetCurrent();
}

void JavaScriptCompiler::AddModule(const std::string name, JavaScriptModule *module)
{
	assert(module);
	modules_[name] = module;
}

Handle<String> JavaScriptCompiler::LoadJavaScriptSource(const std::string &path, bool isLib)
{
        const char *exports_start = "(function(exports) {";
        const char *exports_end  = "});";
        HandleScope scope(isolate_);

        FILE *file = fopen(path.c_str(), "rb");
        if (file == NULL) {
                return Handle<String>();
        }

        fseek(file, 0, SEEK_END);
        size_t bytesOfFile = ftell(file);
        rewind(file);

        size_t total;
        if (isLib) {
                total = bytesOfFile + strlen(exports_start) + strlen(exports_end);
        } else {
                total = bytesOfFile;
        }

        char *buf = new char[total+1];
        buf[total] = '\0';

        if (isLib) {
                strncpy(buf, exports_start, strlen(exports_start));
                buf += strlen(exports_start);
                strncpy(buf + bytesOfFile, exports_end, strlen(exports_end));
        }

        size_t remaining = bytesOfFile;
        char *cp = buf;
        while (remaining)
        {
                size_t bytesOfRead = fread(cp, 1, bytesOfFile, file);
                remaining -= bytesOfRead;
                cp += bytesOfRead;
        }
        
        if (isLib) {
                buf -= strlen(exports_start);
        }

        Handle<String> source = String::New( buf, total);
        delete [] buf;
        fclose(file);
	
        return scope.Close(source);
}

void JavaScriptCompiler::Load(const std::string &AppPath, const std::string &prefix)
{
        HandleScope scope(GetIsolate());
        const std::string path = AppPath + prefix + "/";
	
        filesystem::path ctrolsPath(path);
        filesystem::directory_iterator end_itr;
        for (filesystem::directory_iterator itr(ctrolsPath); itr != end_itr; ++itr)
        {
                if (itr->path().extension().string() == ".js")
                {
                       JavaScriptSource *jss = new JavaScriptSource();
                       jss->path_         = path + itr->path().filename().string();
                       jss->id_           = filesystem::change_extension(itr->path().filename(), "").string();
                       jss->lastModified_ = last_write_time(itr->path());
                       jss->ctx_.Reset(GetIsolate(), Context::New(GetIsolate()));
			
                       Handle<String> source = LoadJavaScriptSource(jss->path_);

                       Handle<Context> ctx = Local<Context>::New(isolate_, jss->ctx_);
			
		       Context::Scope scope_ctx(ctx);
		       BOOST_FOREACH(ModulePair_t p, modules_)
		       {
     				Handle<Object> Obj = p.second->Wrap();
		     	        ctx->Global()->Set(String::New(p.first.c_str()), Obj);
		       }
		  
                       if (ExecuteScript(source).IsEmpty()) {
                               fprintf(stderr, "error in %s", jss->path_.c_str());
			       _exit(1);
                       }
                       sources_[jss->id_] = jss;
                }
        }
}

Handle<Value> JavaScriptCompiler::ExecuteScript(Handle<String> source)
{
	HandleScope scope(isolate_);
        TryCatch try_catch;
        Local<Script> script = Script::Compile(source);
        if (script.IsEmpty()) {
 	    ReportException(isolate_, &try_catch);
            return Handle<Value>();
         }

         Local<Value> result = script->Run();
         if (result.IsEmpty()) {
	     ReportException(isolate_, &try_catch);
             return Handle<Value>();
         }
         return scope.Close(result);;
}

void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope(isolate);
  v8::String::Utf8Value exception(try_catch->Exception());
  const char* exception_string = ToCString(exception);
  v8::Handle<v8::Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    fprintf(stderr, "%s\n", exception_string);
  } else {
    // Print (filename):(line number): (message).
    v8::String::Utf8Value filename(message->GetScriptResourceName());
    const char* filename_string = ToCString(filename);
    int linenum = message->GetLineNumber();
    fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
    // Print line of source code.
    v8::String::Utf8Value sourceline(message->GetSourceLine());
    const char* sourceline_string = ToCString(sourceline);
    fprintf(stderr, "%s\n", sourceline_string);
    // Print wavy underline (GetUnderline is deprecated).
    int start = message->GetStartColumn();
    for (int i = 0; i < start; i++) {
      fprintf(stderr, " ");
    }
    int end = message->GetEndColumn();
    for (int i = start; i < end; i++) {
      fprintf(stderr, "^");
    }
    fprintf(stderr, "\n");
    v8::String::Utf8Value stack_trace(try_catch->StackTrace());
    if (stack_trace.length() > 0) {
      const char* stack_trace_string = ToCString(stack_trace);
      fprintf(stderr, "%s\n", stack_trace_string);
    }
  }
}
