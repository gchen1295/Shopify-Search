#include <nan.h>
#include "searcher.h"
#include "verify.h"
#include <iostream>

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::Object;
using v8::String;
using v8::Value;

class FindProductLocalWorker : public Nan::AsyncWorker
{
public:
  FindProductLocalWorker(Nan::Callback *callback, std::string domain, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, bool randomNotFound, std::string jwt_token) : AsyncWorker(callback), domain(domain), keywords(keywords), negKeywords(negKeywords), size(size), jwt_token(jwt_token), randomNotFound(randomNotFound){};
  ~FindProductLocalWorker(){};

  void Execute()
  {
    bool verified = false;
    verified = JWT::verify(jwt_token);
    if (verified)
    {
      Found fp = Searcher::findByJSON(domain, keywords, negKeywords, size, randomNotFound);
      if (fp.error != "")
      {
        this->SetErrorMessage(fp.error.c_str());
      }
      else
      {
        resp = fp;
      }
    }
    else
    {
      this->SetErrorMessage("Keep trying bozo...");
    }
  }

  void HandleOKCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Object> jsonObj = Nan::New<v8::Object>();

    v8::Local<v8::String> titleProp = Nan::New("productTitle").ToLocalChecked();
    v8::Local<v8::String> handleProp = Nan::New("productHandle").ToLocalChecked();
    v8::Local<v8::String> imageProp = Nan::New("productImage").ToLocalChecked();
    v8::Local<v8::String> priceProp = Nan::New("price").ToLocalChecked();
    v8::Local<v8::String> variantProp = Nan::New("variantID").ToLocalChecked();
    v8::Local<v8::String> sizeProp = Nan::New("size").ToLocalChecked();

    v8::Local<v8::Value> titleVal = Nan::New(resp.productTitle).ToLocalChecked();
    v8::Local<v8::Value> handleVal = Nan::New(resp.productHandle).ToLocalChecked();
    v8::Local<v8::Value> imageVal = Nan::New(resp.productImage).ToLocalChecked();
    v8::Local<v8::Value> priceVal = Nan::New(resp.productPrice).ToLocalChecked();
    v8::Local<v8::Value> variantVal = Nan::New(resp.variantID).ToLocalChecked();
    v8::Local<v8::Value> sizeVal = Nan::New(resp.size).ToLocalChecked();

    Nan::Set(jsonObj, titleProp, titleVal);
    Nan::Set(jsonObj, handleProp, handleVal);
    Nan::Set(jsonObj, imageProp, imageVal);
    Nan::Set(jsonObj, priceProp, priceVal);
    Nan::Set(jsonObj, variantProp, variantVal);
    Nan::Set(jsonObj, sizeProp, sizeVal);

    Local<Value> argv[] = {Nan::Null(), jsonObj}; //Nan::Null() indicates no error
    callback->Call(2, argv, async_resource);      //First parameter is number of arguments passed back
  }

  void HandleErrorCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Value> argv[] = {Nan::New(this->ErrorMessage()).ToLocalChecked(), Nan::Null()};
    callback->Call(2, argv, async_resource);
  }

private:
  std::string domain;
  std::vector<std::string> keywords;
  std::vector<std::string> negKeywords;
  std::string size;
  std::string jwt_token;
  bool randomNotFound;
  Found resp;
};

void FindByJsonLocal(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  try
  {
    if (args.Length() < 7)
    {
      isolate->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, "Invalid number of arguments! => (domain, keywords, negKeywords, size, randomNotFound, jwt_token, callback)", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[0]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Domain must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[1]->IsArray())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Keywords must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[2]->IsArray())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Negative keywords must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[3]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Size must be an string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[4]->IsBoolean())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Random if not found must be an boolean", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[5]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "JWT Token must be an string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[6]->IsFunction())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Callback must be a function", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }

    std::string domain = *Nan::Utf8String(Nan::To<v8::String>(args[0]).ToLocalChecked());
    std::string size = *Nan::Utf8String(Nan::To<v8::String>(args[3]).ToLocalChecked());
    bool randomNotFound = *Nan::Utf8String(Nan::To<v8::Boolean>(args[4]).ToLocalChecked());
    std::string token = *Nan::Utf8String(Nan::To<v8::String>(args[5]).ToLocalChecked());
    std::vector<std::string> keywords;
    std::vector<std::string> negKeywords;
    v8::Local<v8::Array> kwObj = v8::Local<v8::Array>::Cast(args[1]);
    v8::Local<v8::Array> nkwObj = v8::Local<v8::Array>::Cast(args[2]);
    for (size_t i = 0; i < kwObj->Length(); ++i)
    {
      v8::Local<v8::Value> val = Nan::Get(kwObj, i).ToLocalChecked();
      std::string c = *Nan::Utf8String(Nan::To<String>(val).ToLocalChecked());
      keywords.push_back(c);
    }
    for (size_t i = 0; i < nkwObj->Length(); ++i)
    {
      v8::Local<v8::Value> val = Nan::Get(nkwObj, i).ToLocalChecked();
      std::string c = *Nan::Utf8String(Nan::To<String>(val).ToLocalChecked());
      negKeywords.push_back(c);
    }
    Nan::Callback *callback = new Nan::Callback(args[6].As<v8::Function>());

    Nan::AsyncQueueWorker(new FindProductLocalWorker(callback, domain, keywords, negKeywords, size,randomNotFound, token));
  }
  catch (const char *e)
  {
    isolate->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, e, v8::NewStringType::kNormal).ToLocalChecked()));
  }
};

class FindProductWorker : public Nan::AsyncWorker
{
public:
  FindProductWorker(Nan::Callback *callback, std::string domain, std::string proxy, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, bool randomNotFound, std::string jwt_token) : AsyncWorker(callback), domain(domain), keywords(keywords), negKeywords(negKeywords), size(size), jwt_token(jwt_token), proxy(proxy), randomNotFound(randomNotFound){};
  ~FindProductWorker(){};

  void Execute()
  {
    bool verified = false;
    verified = JWT::verify(jwt_token);
    if (verified)
    {
      Found fp = Searcher::findByJSON(domain, proxy, keywords, negKeywords, size, randomNotFound);
      if (fp.error != "")
      {
        this->SetErrorMessage(fp.error.c_str());
      }
      else
      {
        resp = fp;
      }
    }
    else
    {
      this->SetErrorMessage("Keep trying bozo...");
    }
  }

  void HandleOKCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Object> jsonObj = Nan::New<v8::Object>();

    v8::Local<v8::String> titleProp = Nan::New("productTitle").ToLocalChecked();
    v8::Local<v8::String> handleProp = Nan::New("productHandle").ToLocalChecked();
    v8::Local<v8::String> imageProp = Nan::New("productImage").ToLocalChecked();
    v8::Local<v8::String> priceProp = Nan::New("price").ToLocalChecked();
    v8::Local<v8::String> variantProp = Nan::New("variantID").ToLocalChecked();
    v8::Local<v8::String> sizeProp = Nan::New("size").ToLocalChecked();

    v8::Local<v8::Value> titleVal = Nan::New(resp.productTitle).ToLocalChecked();
    v8::Local<v8::Value> handleVal = Nan::New(resp.productHandle).ToLocalChecked();
    v8::Local<v8::Value> imageVal = Nan::New(resp.productImage).ToLocalChecked();
    v8::Local<v8::Value> priceVal = Nan::New(resp.productPrice).ToLocalChecked();
    v8::Local<v8::Value> variantVal = Nan::New(resp.variantID).ToLocalChecked();
    v8::Local<v8::Value> sizeVal = Nan::New(resp.size).ToLocalChecked();

    Nan::Set(jsonObj, titleProp, titleVal);
    Nan::Set(jsonObj, handleProp, handleVal);
    Nan::Set(jsonObj, imageProp, imageVal);
    Nan::Set(jsonObj, priceProp, priceVal);
    Nan::Set(jsonObj, variantProp, variantVal);
    Nan::Set(jsonObj, sizeProp, sizeVal);

    Local<Value> argv[] = {Nan::Null(), jsonObj}; //Nan::Null() indicates no error
    callback->Call(2, argv, async_resource);      //First parameter is number of arguments passed back
  }

  void HandleErrorCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Value> argv[] = {Nan::New(this->ErrorMessage()).ToLocalChecked(), Nan::Null()};
    callback->Call(2, argv, async_resource);
  }

private:
  std::string domain;
  std::string proxy;
  std::vector<std::string> keywords;
  std::vector<std::string> negKeywords;
  std::string size;
  std::string jwt_token;
  bool randomNotFound;
  Found resp;
};

void FindByJson(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  try
  {
    if (args.Length() < 8)
    {
      isolate->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, "Invalid number of arguments! => (domain, proxy, keywords, negKeywords, size, randomNotFound, jwt_token, callback)", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[0]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Domain must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[1]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Proxy must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[2]->IsArray())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Keywords must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[3]->IsArray())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Negative keywords must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[4]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Size must be an string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[5]->IsBoolean())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Random if not found must be a boolean", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[6]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "JWT Token must be an string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[7]->IsFunction())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Callback must be a function", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }

    std::string domain = *Nan::Utf8String(Nan::To<v8::String>(args[0]).ToLocalChecked());
    std::string size = *Nan::Utf8String(Nan::To<v8::String>(args[4]).ToLocalChecked());
    bool randomNotFound = *Nan::Utf8String(Nan::To<v8::Boolean>(args[5]).ToLocalChecked());
    std::string token = *Nan::Utf8String(Nan::To<v8::String>(args[6]).ToLocalChecked());
    std::string proxy = *Nan::Utf8String(Nan::To<v8::String>(args[1]).ToLocalChecked());
    std::vector<std::string> keywords;
    std::vector<std::string> negKeywords;
    v8::Local<v8::Array> kwObj = v8::Local<v8::Array>::Cast(args[2]);
    v8::Local<v8::Array> nkwObj = v8::Local<v8::Array>::Cast(args[3]);
    for (size_t i = 0; i < kwObj->Length(); ++i)
    {
      v8::Local<v8::Value> val = Nan::Get(kwObj, i).ToLocalChecked();
      std::string c = *Nan::Utf8String(Nan::To<String>(val).ToLocalChecked());
      keywords.push_back(c);
    }
    for (size_t i = 0; i < nkwObj->Length(); ++i)
    {
      v8::Local<v8::Value> val = Nan::Get(nkwObj, i).ToLocalChecked();
      std::string c = *Nan::Utf8String(Nan::To<String>(val).ToLocalChecked());
      negKeywords.push_back(c);
    }
    Nan::Callback *callback = new Nan::Callback(args[7].As<v8::Function>());

    Nan::AsyncQueueWorker(new FindProductWorker(callback, domain, proxy, keywords, negKeywords, size,randomNotFound, token));
  }
  catch (const char *e)
  {
    isolate->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, e, v8::NewStringType::kNormal).ToLocalChecked()));
  }
};

class FindHandleLocalM1Worker : public Nan::AsyncWorker
{
public:
  FindHandleLocalM1Worker(Nan::Callback *callback, std::string domain, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, std::string jwt_token) : AsyncWorker(callback), domain(domain), keywords(keywords), negKeywords(negKeywords), size(size), jwt_token(jwt_token){};
  ~FindHandleLocalM1Worker(){};

  void Execute()
  {
    bool verified = false;
    verified = JWT::verify(jwt_token);
    if (verified)
    {
      Found fp = Searcher::findByHandle(domain, keywords, negKeywords, size);
      if (fp.error != "")
      {
        this->SetErrorMessage(fp.error.c_str());
      }
      else
      {
        resp = fp;
      }
    }
    else
    {
      this->SetErrorMessage("Try again bozo....");
    }
  }

  void HandleOKCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Object> jsonObj = Nan::New<v8::Object>();

    v8::Local<v8::String> titleProp = Nan::New("productTitle").ToLocalChecked();
    v8::Local<v8::String> handleProp = Nan::New("productHandle").ToLocalChecked();
    v8::Local<v8::String> imageProp = Nan::New("productImage").ToLocalChecked();
    v8::Local<v8::String> urlProp = Nan::New("productURL").ToLocalChecked();
    v8::Local<v8::String> priceProp = Nan::New("price").ToLocalChecked();
    v8::Local<v8::String> variantProp = Nan::New("variantID").ToLocalChecked();
    v8::Local<v8::String> sizeProp = Nan::New("size").ToLocalChecked();

    v8::Local<v8::Value> titleVal = Nan::New(resp.productTitle).ToLocalChecked();
    v8::Local<v8::Value> handleVal = Nan::New(resp.productHandle).ToLocalChecked();
    v8::Local<v8::Value> imageVal = Nan::New(resp.productImage).ToLocalChecked();
    v8::Local<v8::Value> urlVal = Nan::New(resp.productURL).ToLocalChecked();
    v8::Local<v8::Value> priceVal = Nan::New(resp.productPrice).ToLocalChecked();
    v8::Local<v8::Value> variantVal = Nan::New(resp.variantID).ToLocalChecked();
    v8::Local<v8::Value> sizeVal = Nan::New(resp.size).ToLocalChecked();

    Nan::Set(jsonObj, titleProp, titleVal);
    Nan::Set(jsonObj, handleProp, handleVal);
    Nan::Set(jsonObj, imageProp, imageVal);
    Nan::Set(jsonObj, urlProp, urlVal);
    Nan::Set(jsonObj, priceProp, priceVal);
    Nan::Set(jsonObj, variantProp, variantVal);
    Nan::Set(jsonObj, sizeProp, sizeVal);

    Local<Value> argv[] = {Nan::Null(), jsonObj}; //Nan::Null() indicates no error
    callback->Call(2, argv, async_resource);      //First parameter is number of arguments passed back
  }

  void HandleErrorCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Value> argv[] = {Nan::New(this->ErrorMessage()).ToLocalChecked(), Nan::Null()};
    callback->Call(2, argv, async_resource);
  }

private:
  std::string domain;
  std::vector<std::string> keywords;
  std::vector<std::string> negKeywords;
  std::string size;
  std::string jwt_token;
  Found resp;
};

void FindByHandleLocalM1(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  try
  {
    if (args.Length() < 6)
    {
      isolate->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, "Invalid number of arguments! => (domain, keywords, negKeywords, size, jwt_token, callback)", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[0]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Domain must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[1]->IsArray())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Keywords must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[2]->IsArray())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Negative keywords must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[3]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Size must be an string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[4]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "JWT Token must be an string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[5]->IsFunction())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Callback must be a function", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }

    std::string domain = *Nan::Utf8String(Nan::To<v8::String>(args[0]).ToLocalChecked());
    std::string size = *Nan::Utf8String(Nan::To<v8::String>(args[3]).ToLocalChecked());
    std::string token = *Nan::Utf8String(Nan::To<v8::String>(args[4]).ToLocalChecked());
    std::vector<std::string> keywords;
    std::vector<std::string> negKeywords;
    v8::Local<v8::Array> kwObj = v8::Local<v8::Array>::Cast(args[1]);
    v8::Local<v8::Array> nkwObj = v8::Local<v8::Array>::Cast(args[2]);
    for (size_t i = 0; i < kwObj->Length(); ++i)
    {
      v8::Local<v8::Value> val = Nan::Get(kwObj, i).ToLocalChecked();
      std::string c = *Nan::Utf8String(Nan::To<String>(val).ToLocalChecked());
      keywords.push_back(c);
    }
    for (size_t i = 0; i < nkwObj->Length(); ++i)
    {
      v8::Local<v8::Value> val = Nan::Get(nkwObj, i).ToLocalChecked();
      std::string c = *Nan::Utf8String(Nan::To<String>(val).ToLocalChecked());
      negKeywords.push_back(c);
    }
    Nan::Callback *callback = new Nan::Callback(args[5].As<v8::Function>());

    Nan::AsyncQueueWorker(new FindHandleLocalM1Worker(callback, domain, keywords, negKeywords, size, token));
  }
  catch (const char *e)
  {
    isolate->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, e, v8::NewStringType::kNormal).ToLocalChecked()));
  }
};

class FindHandleM1Worker : public Nan::AsyncWorker
{
public:
  FindHandleM1Worker(Nan::Callback *callback, std::string domain, std::string proxy, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, std::string jwt_token) : AsyncWorker(callback), domain(domain), keywords(keywords), negKeywords(negKeywords), size(size), jwt_token(jwt_token), proxy(proxy){};
  ~FindHandleM1Worker(){};

  void Execute()
  {
    bool verified = false;
    verified = JWT::verify(jwt_token);
    if (verified)
    {
      Found fp = Searcher::findByHandle(domain, proxy, keywords, negKeywords, size);
      if (fp.error != "")
      {
        this->SetErrorMessage(fp.error.c_str());
      }
      else
      {
        resp = fp;
      }
    }
    else
    {
      this->SetErrorMessage("Try again bozo....");
    }
  }

  void HandleOKCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Object> jsonObj = Nan::New<v8::Object>();

    v8::Local<v8::String> titleProp = Nan::New("productTitle").ToLocalChecked();
    v8::Local<v8::String> handleProp = Nan::New("productHandle").ToLocalChecked();
    v8::Local<v8::String> imageProp = Nan::New("productImage").ToLocalChecked();
    v8::Local<v8::String> urlProp = Nan::New("productURL").ToLocalChecked();
    v8::Local<v8::String> priceProp = Nan::New("price").ToLocalChecked();
    v8::Local<v8::String> variantProp = Nan::New("variantID").ToLocalChecked();
    v8::Local<v8::String> sizeProp = Nan::New("size").ToLocalChecked();

    v8::Local<v8::Value> titleVal = Nan::New(resp.productTitle).ToLocalChecked();
    v8::Local<v8::Value> handleVal = Nan::New(resp.productHandle).ToLocalChecked();
    v8::Local<v8::Value> imageVal = Nan::New(resp.productImage).ToLocalChecked();
    v8::Local<v8::Value> urlVal = Nan::New(resp.productURL).ToLocalChecked();
    v8::Local<v8::Value> priceVal = Nan::New(resp.productPrice).ToLocalChecked();
    v8::Local<v8::Value> variantVal = Nan::New(resp.variantID).ToLocalChecked();
    v8::Local<v8::Value> sizeVal = Nan::New(resp.size).ToLocalChecked();

    Nan::Set(jsonObj, titleProp, titleVal);
    Nan::Set(jsonObj, handleProp, handleVal);
    Nan::Set(jsonObj, imageProp, imageVal);
    Nan::Set(jsonObj, urlProp, urlVal);
    Nan::Set(jsonObj, priceProp, priceVal);
    Nan::Set(jsonObj, variantProp, variantVal);
    Nan::Set(jsonObj, sizeProp, sizeVal);

    Local<Value> argv[] = {Nan::Null(), jsonObj}; //Nan::Null() indicates no error
    callback->Call(2, argv, async_resource);      //First parameter is number of arguments passed back
  }

  void HandleErrorCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Value> argv[] = {Nan::New(this->ErrorMessage()).ToLocalChecked(), Nan::Null()};
    callback->Call(2, argv, async_resource);
  }

private:
  std::string domain;
  std::string proxy;
  std::vector<std::string> keywords;
  std::vector<std::string> negKeywords;
  std::string size;
  std::string jwt_token;
  Found resp;
};

void FindByHandleM1(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  try
  {
    if (args.Length() < 7)
    {
      isolate->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, "Invalid number of arguments! => (domain, proxy, keywords, negKeywords, size, jwt_token, callback)", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[0]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Domain must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[1]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Proxy must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[2]->IsArray())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Keywords must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[3]->IsArray())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Negative keywords must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[4]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Size must be an string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[5]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "JWT Token must be an string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[6]->IsFunction())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Callback must be a function", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }

    std::string domain = *Nan::Utf8String(Nan::To<v8::String>(args[0]).ToLocalChecked());
    std::string proxy = *Nan::Utf8String(Nan::To<v8::String>(args[1]).ToLocalChecked());
    std::string size = *Nan::Utf8String(Nan::To<v8::String>(args[4]).ToLocalChecked());
    std::string token = *Nan::Utf8String(Nan::To<v8::String>(args[5]).ToLocalChecked());
    std::vector<std::string> keywords;
    std::vector<std::string> negKeywords;
    v8::Local<v8::Array> kwObj = v8::Local<v8::Array>::Cast(args[2]);
    v8::Local<v8::Array> nkwObj = v8::Local<v8::Array>::Cast(args[3]);
    for (size_t i = 0; i < kwObj->Length(); ++i)
    {
      v8::Local<v8::Value> val = Nan::Get(kwObj, i).ToLocalChecked();
      std::string c = *Nan::Utf8String(Nan::To<String>(val).ToLocalChecked());
      keywords.push_back(c);
    }
    for (size_t i = 0; i < nkwObj->Length(); ++i)
    {
      v8::Local<v8::Value> val = Nan::Get(nkwObj, i).ToLocalChecked();
      std::string c = *Nan::Utf8String(Nan::To<String>(val).ToLocalChecked());
      negKeywords.push_back(c);
    }
    Nan::Callback *callback = new Nan::Callback(args[6].As<v8::Function>());

    Nan::AsyncQueueWorker(new FindHandleM1Worker(callback, proxy, domain, keywords, negKeywords, size, token));
  }
  catch (const char *e)
  {
    isolate->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, e, v8::NewStringType::kNormal).ToLocalChecked()));
  }
};

class SiteParserLocalWorker : public Nan::AsyncWorker
{
public:
  SiteParserLocalWorker(Nan::Callback *callback, std::string domain, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, bool randomNotFound, std::string jwt_token) : AsyncWorker(callback), domain(domain), keywords(keywords), negKeywords(negKeywords), size(size), jwt_token(jwt_token), randomNotFound(randomNotFound){};
  ~SiteParserLocalWorker(){};

  void Execute()
  {
    bool verified = false;
    verified = JWT::verify(jwt_token);
    if (verified)
    {
      Found fp = Searcher::findBySiteMap(domain, keywords, negKeywords, size, randomNotFound);
      if (fp.error != "")
      {
        this->SetErrorMessage(fp.error.c_str());
      }
      else
      {
        resp = fp;
      }
    }
    else
    {
      this->SetErrorMessage("Keep trying you bozo...");
    }
  }

  void HandleOKCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Object> jsonObj = Nan::New<v8::Object>();

    v8::Local<v8::String> titleProp = Nan::New("productTitle").ToLocalChecked();
    v8::Local<v8::String> handleProp = Nan::New("productHandle").ToLocalChecked();
    v8::Local<v8::String> imageProp = Nan::New("productImage").ToLocalChecked();
    v8::Local<v8::String> urlProp = Nan::New("productURL").ToLocalChecked();
    v8::Local<v8::String> priceProp = Nan::New("price").ToLocalChecked();
    v8::Local<v8::String> variantProp = Nan::New("variantID").ToLocalChecked();
    v8::Local<v8::String> sizeProp = Nan::New("size").ToLocalChecked();

    v8::Local<v8::Value> titleVal = Nan::New(resp.productTitle).ToLocalChecked();
    v8::Local<v8::Value> handleVal = Nan::New(resp.productHandle).ToLocalChecked();
    v8::Local<v8::Value> imageVal = Nan::New(resp.productImage).ToLocalChecked();
    v8::Local<v8::Value> urlVal = Nan::New(resp.productURL).ToLocalChecked();
    v8::Local<v8::Value> priceVal = Nan::New(resp.productPrice).ToLocalChecked();
    v8::Local<v8::Value> variantVal = Nan::New(resp.variantID).ToLocalChecked();
    v8::Local<v8::Value> sizeVal = Nan::New(resp.size).ToLocalChecked();

    Nan::Set(jsonObj, titleProp, titleVal);
    Nan::Set(jsonObj, handleProp, handleVal);
    Nan::Set(jsonObj, imageProp, imageVal);
    Nan::Set(jsonObj, urlProp, urlVal);
    Nan::Set(jsonObj, priceProp, priceVal);
    Nan::Set(jsonObj, variantProp, variantVal);
    Nan::Set(jsonObj, sizeProp, sizeVal);

    Local<Value> argv[] = {Nan::Null(), jsonObj}; //Nan::Null() indicates no error
    callback->Call(2, argv, async_resource);      //First parameter is number of arguments passed back
  }

  void HandleErrorCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Value> argv[] = {Nan::New(this->ErrorMessage()).ToLocalChecked(), Nan::Null()};
    callback->Call(2, argv, async_resource);
  }

private:
  std::string domain;
  std::vector<std::string> keywords;
  std::vector<std::string> negKeywords;
  std::string size;
  std::string jwt_token;
  bool randomNotFound;
  Found resp;
};

void FindBySiteMapLocal(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  try
  {
    if (args.Length() < 7)
    {
      isolate->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, "Invalid number of arguments! => (domain, keywords, negKeywords, size, randomNotFound, jwt_token, callback)", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[0]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Domain must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[1]->IsArray())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Keywords must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[2]->IsArray())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Negative keywords must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[3]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Size must be an string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[4]->IsBoolean())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Random if not found must be a boolean", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[5]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "JWT Token must be an string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[6]->IsFunction())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Callback must be a function", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }

    std::string domain = *Nan::Utf8String(Nan::To<v8::String>(args[0]).ToLocalChecked());
    std::string size = *Nan::Utf8String(Nan::To<v8::String>(args[3]).ToLocalChecked());
    bool randomNotFound = *Nan::Utf8String(Nan::To<v8::Boolean>(args[4]).ToLocalChecked());
    std::string token = *Nan::Utf8String(Nan::To<v8::String>(args[5]).ToLocalChecked());
    std::vector<std::string> keywords;
    std::vector<std::string> negKeywords;
    v8::Local<v8::Array> kwObj = v8::Local<v8::Array>::Cast(args[1]);
    v8::Local<v8::Array> nkwObj = v8::Local<v8::Array>::Cast(args[2]);
    for (size_t i = 0; i < kwObj->Length(); ++i)
    {
      v8::Local<v8::Value> val = Nan::Get(kwObj, i).ToLocalChecked();
      std::string c = *Nan::Utf8String(Nan::To<String>(val).ToLocalChecked());
      keywords.push_back(c);
    }
    for (size_t i = 0; i < nkwObj->Length(); ++i)
    {
      v8::Local<v8::Value> val = Nan::Get(nkwObj, i).ToLocalChecked();
      std::string c = *Nan::Utf8String(Nan::To<String>(val).ToLocalChecked());
      negKeywords.push_back(c);
    }
    Nan::Callback *callback = new Nan::Callback(args[6].As<v8::Function>());

    Nan::AsyncQueueWorker(new SiteParserLocalWorker(callback, domain, keywords, negKeywords, size,randomNotFound, token));
  }
  catch (const char *e)
  {
    isolate->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, e, v8::NewStringType::kNormal).ToLocalChecked()));
  }
};

class SiteParserWorker : public Nan::AsyncWorker
{
public:
  SiteParserWorker(Nan::Callback *callback, std::string domain, std::string proxy,std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, bool randomNotFound, std::string jwt_token) : AsyncWorker(callback), domain(domain), keywords(keywords), negKeywords(negKeywords), size(size), jwt_token(jwt_token), proxy(proxy), randomNotFound(randomNotFound){};
  ~SiteParserWorker(){};

  void Execute()
  {
    bool verified = false;
    verified = JWT::verify(jwt_token);
    if (verified)
    {
      Found fp = Searcher::findBySiteMap(domain, proxy, keywords, negKeywords, size, randomNotFound);
      if (fp.error != "")
      {
        this->SetErrorMessage(fp.error.c_str());
      }
      else
      {
        resp = fp;
      }
    }
    else
    {
      this->SetErrorMessage("Keep trying you bozo...");
    }
  }

  void HandleOKCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Object> jsonObj = Nan::New<v8::Object>();

    v8::Local<v8::String> titleProp = Nan::New("productTitle").ToLocalChecked();
    v8::Local<v8::String> handleProp = Nan::New("productHandle").ToLocalChecked();
    v8::Local<v8::String> imageProp = Nan::New("productImage").ToLocalChecked();
    v8::Local<v8::String> urlProp = Nan::New("productURL").ToLocalChecked();
    v8::Local<v8::String> priceProp = Nan::New("price").ToLocalChecked();
    v8::Local<v8::String> variantProp = Nan::New("variantID").ToLocalChecked();
    v8::Local<v8::String> sizeProp = Nan::New("size").ToLocalChecked();

    v8::Local<v8::Value> titleVal = Nan::New(resp.productTitle).ToLocalChecked();
    v8::Local<v8::Value> handleVal = Nan::New(resp.productHandle).ToLocalChecked();
    v8::Local<v8::Value> imageVal = Nan::New(resp.productImage).ToLocalChecked();
    v8::Local<v8::Value> urlVal = Nan::New(resp.productURL).ToLocalChecked();
    v8::Local<v8::Value> priceVal = Nan::New(resp.productPrice).ToLocalChecked();
    v8::Local<v8::Value> variantVal = Nan::New(resp.variantID).ToLocalChecked();
    v8::Local<v8::Value> sizeVal = Nan::New(resp.size).ToLocalChecked();

    Nan::Set(jsonObj, titleProp, titleVal);
    Nan::Set(jsonObj, handleProp, handleVal);
    Nan::Set(jsonObj, imageProp, imageVal);
    Nan::Set(jsonObj, urlProp, urlVal);
    Nan::Set(jsonObj, priceProp, priceVal);
    Nan::Set(jsonObj, variantProp, variantVal);
    Nan::Set(jsonObj, sizeProp, sizeVal);

    Local<Value> argv[] = {Nan::Null(), jsonObj}; //Nan::Null() indicates no error
    callback->Call(2, argv, async_resource);      //First parameter is number of arguments passed back
  }

  void HandleErrorCallback()
  {
    Nan::HandleScope scope;
    v8::Local<v8::Value> argv[] = {Nan::New(this->ErrorMessage()).ToLocalChecked(), Nan::Null()};
    callback->Call(2, argv, async_resource);
  }

private:
  std::string domain;
  std::string proxy;
  std::vector<std::string> keywords;
  std::vector<std::string> negKeywords;
  std::string size;
  std::string jwt_token;
  bool randomNotFound;
  Found resp;
};

void FindBySiteMap(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  try
  {
    if (args.Length() < 8)
    {
      isolate->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, "Invalid number of arguments! => (domain, proxy, keywords, negKeywords, size, randomNotFound, jwt_token, callback)", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[0]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Domain must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[1]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Proxy must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[2]->IsArray())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Keywords must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[3]->IsArray())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Negative keywords must be a string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[4]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Size must be an string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[5]->IsBoolean())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Random if not found must be a boolean", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[6]->IsString())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "JWT Token must be an string", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }
    if (!args[7]->IsFunction())
    {
      isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Callback must be a function", v8::NewStringType::kNormal).ToLocalChecked()));
      return;
    }

    std::string domain = *Nan::Utf8String(Nan::To<v8::String>(args[0]).ToLocalChecked());
    std::string proxy = *Nan::Utf8String(Nan::To<v8::String>(args[1]).ToLocalChecked());
    std::string size = *Nan::Utf8String(Nan::To<v8::String>(args[4]).ToLocalChecked());
    bool randomNotFound = *Nan::Utf8String(Nan::To<v8::Boolean>(args[5]).ToLocalChecked());
    std::string token = *Nan::Utf8String(Nan::To<v8::String>(args[6]).ToLocalChecked());
    std::vector<std::string> keywords;
    std::vector<std::string> negKeywords;
    v8::Local<v8::Array> kwObj = v8::Local<v8::Array>::Cast(args[2]);
    v8::Local<v8::Array> nkwObj = v8::Local<v8::Array>::Cast(args[3]);
    for (size_t i = 0; i < kwObj->Length(); ++i)
    {
      v8::Local<v8::Value> val = Nan::Get(kwObj, i).ToLocalChecked();
      std::string c = *Nan::Utf8String(Nan::To<String>(val).ToLocalChecked());
      keywords.push_back(c);
    }
    for (size_t i = 0; i < nkwObj->Length(); ++i)
    {
      v8::Local<v8::Value> val = Nan::Get(nkwObj, i).ToLocalChecked();
      std::string c = *Nan::Utf8String(Nan::To<String>(val).ToLocalChecked());
      negKeywords.push_back(c);
    }
    Nan::Callback *callback = new Nan::Callback(args[7].As<v8::Function>());
    Nan::AsyncQueueWorker(new SiteParserWorker(callback, domain, proxy, keywords, negKeywords, size, randomNotFound, token));
  }
  catch (const char *e)
  {
    isolate->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, e, v8::NewStringType::kNormal).ToLocalChecked()));
  }
};

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module)
{
  NODE_SET_METHOD(exports, "findByJSONLocal", FindByJsonLocal);
  NODE_SET_METHOD(exports, "findByHandleLocal", FindByHandleLocalM1);
  NODE_SET_METHOD(exports, "findBySiteMapLocal", FindBySiteMapLocal);
  NODE_SET_METHOD(exports, "findByJSON", FindByJson);
  NODE_SET_METHOD(exports, "findByHandle", FindByHandleM1);
  NODE_SET_METHOD(exports, "findBySiteMap", FindBySiteMap);
}

NODE_MODULE(searcher, Init)