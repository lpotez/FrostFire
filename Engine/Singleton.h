#pragma once

namespace FrostFireEngine
{
  template <class T>
  class CSingleton {
  public:
    static T& GetInstance()
    {
      static T instance;
      return instance;
    }

    CSingleton(const CSingleton&) = delete;
    CSingleton& operator=(const CSingleton&) = delete;

  protected:
    CSingleton() = default;
    virtual ~CSingleton() = default;
  };
}
