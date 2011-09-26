#include <unistd.h>
#include <stdio.h>
#include <v8.h>
#include <node.h>
#include "combine.h"

using namespace node;
using namespace v8;
using namespace std;


#define REQ_FUN_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsFunction())                   \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be a function")));  \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

#define REQ_STR_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsString())                   \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be a string")));  \
  Local<String> VAR = Local<String>::Cast(args[I]);

#define REQ_INT_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsNumber())                   \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be a number")));  \
  Local<Int32> VAR = Local<Int32>::Cast(args[I]);


class NativeImage: ObjectWrap
{
  
  //////////////////////////////////////////////////
  // Class definition, etc
  //////////////////////////////////////////////////  
  
private:
  int m_count;
public:

  static Persistent<FunctionTemplate> s_ct;
  // called once when module is required
  static void Init(Handle<Object> target)
  {
    HandleScope scope;
    // create the NativeImage Javascript function/class template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    s_ct = Persistent<FunctionTemplate>::New(t);
    s_ct->InstanceTemplate()->SetInternalFieldCount(1);
    s_ct->SetClassName(String::NewSymbol("NativeImage"));
    // attach the "combine4" instance method
    NODE_SET_PROTOTYPE_METHOD(s_ct, "combine4", Combine4);
    // attach module.NativeImage
    target->Set(String::NewSymbol("NativeImage"),s_ct->GetFunction());
  }

  NativeImage() :
    m_count(0)
  {
  }

  ~NativeImage()
  {
  }
  
  // This becomes the Javascript constructor
  static Handle<Value> New(const Arguments& args)
  {
    HandleScope scope;
    NativeImage* ni = new NativeImage();
    ni->Wrap(args.This());
    return args.This();
  }
  
  //////////////////////////////////////////////////
  // combine4 method
  //////////////////////////////////////////////////
  
  // this map is used to pass values to the new thread
  // that we will create ( to keep this method async )
  struct combine4_baton_t {
    NativeImage *ni;
    v8::Persistent<v8::String> img1;
    v8::Persistent<v8::String> img2;
    v8::Persistent<v8::String> img3;
    v8::Persistent<v8::String> img4;
    v8::Persistent<v8::String> imgdest;
    v8::Persistent<v8::Function> cb;
  };

  // STEP 1
  // this is where the method call enters from javascript
  // we extract arguments, put them on a struct,
  // and create a new thread ( libeio )
  static Handle<Value> Combine4(const Arguments& args)
  {
    HandleScope scope;
    
    // extract arguments
    REQ_STR_ARG(0, img1);
    REQ_STR_ARG(1, img2);
    REQ_STR_ARG(2, img3);
    REQ_STR_ARG(3, img4);
    REQ_STR_ARG(4, imgdest);
    REQ_FUN_ARG(5, cb);

    NativeImage* ni = ObjectWrap::Unwrap<NativeImage>(args.This());

    combine4_baton_t *baton = new combine4_baton_t();
    baton->ni = ni;
    
    // store arguments and callback as persistent handles
    // so they survive this context
    baton->img1 = v8::Persistent<v8::String>::New( img1 );
    baton->img2 = v8::Persistent<v8::String>::New( img2 );
    baton->img3 = v8::Persistent<v8::String>::New( img3 );
    baton->img4 = v8::Persistent<v8::String>::New( img4 );
    baton->imgdest = v8::Persistent<v8::String>::New( imgdest );
    baton->cb = v8::Persistent<v8::Function>::New( cb );

    // increate reference count to "this" object to prevent
    // it from being GC'd while the other thread is running
    ni->Ref();

    // start a new thread with libeio. pass struct with arguments
    eio_custom(EIO_Combine4, EIO_PRI_DEFAULT, EIO_AfterCombine4, baton);
    // Increase reference count to the event loop
    // Node.js would exit if there is nothing left
    ev_ref(EV_DEFAULT_UC);

    // this method returns undefined because work will actually
    // be done in another thread
    return Undefined();
  }

  // STEP 2
  // This method is run on a new thread
  // We recover the data passed via a struct
  // And perform our work
  static int EIO_Combine4(eio_req *req)
  {
    combine4_baton_t *baton = static_cast<combine4_baton_t *>(req->data);
    
    v8::String::Utf8Value img1(baton->img1);
    v8::String::Utf8Value img2(baton->img2);
    v8::String::Utf8Value img3(baton->img3);
    v8::String::Utf8Value img4(baton->img4);
    v8::String::Utf8Value imgdest(baton->imgdest);
    
    /*
    printf("Combine4 %s %s %s %s %s",  *img1, *img2, *img3, *img4, *imgdest);
    printf("\n");
    fflush(stdout);
    */
    
    /*
    char* img1 = "/usr/src/fast-pyramid-generator/test_images/checkers/0_0.png";
    char* img2 = "/usr/src/fast-pyramid-generator/test_images/checkers/1_0.png";
    char* img3 = "/usr/src/fast-pyramid-generator/test_images/checkers/0_1.png";
    char* img4 = "/usr/src/fast-pyramid-generator/test_images/checkers/1_1.png";
    char* dest = "/tmp/test.jpg";
    */
    combine4( *img1, *img2, *img3, *img4, *imgdest );

    return 0;
  }

  // STEP 3
  // The spawned thread has finished and we returned to main
  // Time to grab the callback and pass the results
  static int EIO_AfterCombine4(eio_req *req)
  {
    HandleScope scope;
    combine4_baton_t *baton = static_cast<combine4_baton_t *>(req->data);
    ev_unref(EV_DEFAULT_UC);
    baton->ni->Unref();

    Local<Value> argv[1];

    argv[0] = String::New( "OK" );
    TryCatch try_catch;
    baton->cb->Call(Context::GetCurrent()->Global(), 1, argv);
    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
    
    baton->cb.Dispose();
    baton->img1.Dispose();
    baton->img2.Dispose();
    baton->img3.Dispose();
    baton->img4.Dispose();
    baton->imgdest.Dispose();
    
    delete baton;
    return 0;
  }
};

Persistent<FunctionTemplate> NativeImage::s_ct;
extern "C" {
  static void init (Handle<Object> target)
  {
    NativeImage::Init(target);
  }
  NODE_MODULE(nativeimage, init);
}
