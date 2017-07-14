#include "Scene.h"
#include "SceneItem.h"
#include "Input.h"

namespace osn {

obs::source Scene::GetHandle()
{
    return handle.get().get();
}

Scene::Scene(std::string name)
 : handle(obs::scene(name))
{
}

Scene::Scene(obs::scene handle)
 : handle(obs::scene(handle))
{
}

Nan::Persistent<v8::FunctionTemplate> Scene::prototype =
    Nan::Persistent<v8::FunctionTemplate>();

NAN_MODULE_INIT(Scene::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->Inherit(Nan::New(ISource::prototype));
    locProto->SetClassName(FIELD_NAME("Scene"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("source"), source);
    Nan::SetMethod(locProto->PrototypeTemplate(), "getItems", getItems);
    Nan::SetMethod(locProto->PrototypeTemplate(), "add", add);
    Nan::SetMethod(locProto, "getCurrentListDeprecated", getCurrentListDeprecated);
    Nan::SetMethod(locProto, "create", create);
    Nan::SetMethod(locProto, "fromName", fromName);

    Nan::Set(target, FIELD_NAME("Scene"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_GETTER(Scene::source)
{
    obs::weak<obs::scene> &scene = Scene::Object::GetHandle(info.Holder());

    Input *binding = new Input(scene.get()->source());
    auto object = Input::Object::GenerateObject(binding);

    info.GetReturnValue().Set(object);
}

NAN_METHOD(Scene::getCurrentListDeprecated)
{
    std::vector<obs_source_t *> sources;

    auto cb = [] (void *data, obs_source_t *source) -> bool {
        std::vector<obs_source_t *> *sources = 
            reinterpret_cast<std::vector<obs_source_t *>*>(data);

        if (obs_source_get_type(source) == OBS_SOURCE_TYPE_SCENE)
            sources->push_back(obs_source_get_ref(source));

        return true;
    };

    obs_enum_sources(cb, &sources);
    auto array = Nan::New<v8::Array>(static_cast<int>(sources.size()));

    for (int i = 0; i < sources.size(); ++i) {
        Scene *binding = new Scene(obs::scene(sources[i]));
        auto object = Scene::Object::GenerateObject(binding);
        Nan::Set(array, i, object);
    }

    info.GetReturnValue().Set(array);
}

NAN_METHOD(Scene::create)
{
    ASSERT_INFO_LENGTH(info, 1);
    
    std::string name;

    ASSERT_GET_VALUE(info[0], name);

    Scene *binding = new Scene(name);
    auto object = Scene::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}


NAN_METHOD(Scene::fromName)
{
    ASSERT_INFO_LENGTH(info, 1);

    std::string name;

    ASSERT_GET_VALUE(info[0], name);

    Scene *binding = new Scene(obs::scene::from_name(name));
    auto object = Scene::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Scene::getItems)
{
    obs::weak<obs::scene> &scene = Scene::Object::GetHandle(info.Holder());
    std::vector<obs::scene::item> items = scene.get()->items();
    int size = static_cast<int>(items.size());
    auto array = Nan::New<v8::Array>(size);

    for (int i = 0; i < size; ++i) {
        SceneItem *binding = new SceneItem(items[i]);
        auto object = SceneItem::Object::GenerateObject(binding);
        Nan::Set(array, i, object);
    }

    info.GetReturnValue().Set(array);
}

NAN_METHOD(Scene::add)
{
    obs::weak<obs::scene> &scene = Scene::Object::GetHandle(info.Holder());

    ASSERT_INFO_LENGTH(info, 1);

    v8::Local<v8::Object> input_obj;

    ASSERT_GET_VALUE(info[0], input_obj);

    obs::weak<obs::input> &input = 
        Input::Object::GetHandle(input_obj);

    SceneItem *binding = new SceneItem(scene.get()->add(input.get().get()));
    auto object = SceneItem::Object::GenerateObject(binding);

    info.GetReturnValue().Set(object);
}

}