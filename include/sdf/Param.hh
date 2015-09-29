/*
 * Copyright 2012 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef _SDF_PARAM_HH_
#define _SDF_PARAM_HH_

#include <memory>
#include <functional>
#include <algorithm>
#include <typeinfo>
#include <string>
#include <vector>
#include <ignition/math.hh>

#include "sdf/Console.hh"
#include "sdf/system_util.hh"

/// \todo Remove this diagnositic push/pop in version 5
#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include "sdf/Types.hh"
#include "sdf/Util.hh"
#ifndef _WIN32
#pragma GCC diagnostic pop
#endif

namespace sdf
{
  class SDFORMAT_VISIBLE Param;

  /// \def ParamPtr
  /// \brief Shared pointer to a Param
  typedef std::shared_ptr<Param> ParamPtr;

  /// \def Param_V
  /// \brief vector of shared pointers to a Param
  typedef std::vector<ParamPtr> Param_V;

  /// \internal
  class ParamPrivate;

  /// \class Param Param.hh sdf/sdf.hh
  /// \brief A parameter class
  class SDFORMAT_VISIBLE Param
  {
    /// \brief Constructor.
    /// \param[in] _key Key for the parameter.
    /// \param[in] _typeName String name for the value type (double,
    /// int,...).
    /// \param[in] _default Default value.
    /// \param[in] _required True if the parameter is required to be set.
    /// \param[in] _description Description of the parameter.
    public: Param(const std::string &_key, const std::string &_typeName,
                  const std::string &_default, bool _required,
                  const std::string &_description = "");

    /// \brief Destructor
    public: virtual ~Param();

    /// \brief Get the value as a string.
    /// \return String containing the value of the parameter.
    public: std::string GetAsString() const;

    /// \brief Get the default value as a string.
    /// \return String containing the default value of the parameter.
    public: std::string GetDefaultAsString() const;

    /// \brief Set the parameter value from a string.
    /// \param[in] _value New value for the parameter in string form.
    public: bool SetFromString(const std::string &_value);

    /// \brief Reset the parameter to the default value.
    public: void Reset();

    /// \brief Get the key value.
    /// \return The key.
    public: const std::string &GetKey() const;

    /// \brief Get the type of the value stored.
    /// \return The std::type_info.
    /// \deprecated GetType is unstable. Use IsType().
    /// \sa IsType
    public: const std::type_info &GetType() const SDF_DEPRECATED(4.0);

    /// \brief Return true if the param is a particular type
    /// \return True if the type held by this Param matches the Type
    /// template parameter.
    public: template<typename T>
            bool IsType() const;

    /// \brief Get the type name value.
    /// \return The type name.
    public: const std::string &GetTypeName() const;

    /// \brief Return whether the parameter is required.
    /// \return True if the parameter is required.
    public: bool GetRequired() const;

    /// \brief Return true if the parameter has been set.
    /// \return True if the parameter has been set.
    public: bool GetSet() const;

    /// \brief Clone the parameter.
    /// \return A new parameter that is the clone of this.
    public: ParamPtr Clone() const;

    /// \brief Set the update function. The updateFunc will be used to
    /// set the parameter's value when Param::Update is called.
    /// \param[in] _updateFunc Function pointer to an update function.
    public: template<typename T>
            void SetUpdateFunc(T _updateFunc);

    /// \brief Set the parameter's value using the updateFunc.
    /// \sa Param::SetUpdateFunc
    public: void Update();

    /// \brief Set the parameter's value.
    ///
    /// This means the value must have an input and output stream operator.
    /// \param[in] _value The value to set the parameter to.
    /// \return True if the value was successfully set.
    public: template<typename T>
            bool Set(const T &_value);

    /// \brief Get the value of the parameter.
    /// \param[out] _value The value of the parameter.
    /// \return True if parameter was successfully cast to the value type
    /// passed in.
    public: template<typename T>
            bool Get(T &_value) const;

    /// \brief Get the default value of the parameter.
    /// \param[out] _value The default value of the parameter.
    /// \return True if parameter was successfully cast to the value type
    /// passed in.
    public: template<typename T>
            bool GetDefault(T &_value) const;

    /// \brief Equal operator. Set's the value and default value from the
    /// provided Param.
    /// \param[in] _param The parameter to set values from.
    /// \return *This
    public: Param &operator=(const Param &_param);

    /// \brief Set the description of the parameter.
    /// \param[in] _desc New description for the parameter.
    public: void SetDescription(const std::string &_desc);

    /// \brief Get the description of the parameter.
    /// \return The description of the parameter.
    public: std::string GetDescription() const;

    /// \brief Ostream operator. Outputs the parameter's value.
    /// \param[in] _out Output stream.
    /// \param[in] _p The parameter to output.
    /// \return The output stream.
    public: friend std::ostream &operator<<(std::ostream &_out,
                                            const Param &_p)
            {
              _out << _p.GetAsString();
              return _out;
            }

    /// \brief Initialize the value. This is called from the constructor.
    /// \param[in] _value Value to set the parameter to.
    private: template<typename T>
             void Init(const std::string &_value);


    /// \internal
    /// \brief A custom version of an any class.
    private: class Some
    {
      /// \brief Used for RTTI
      private: using id = size_t;

      /// \brief Used by the equal operator
      private: template<typename T>
               using decay = typename std::decay<T>::type;

      /// \brief Used by the equal operator to make sure a Some's value
      /// does not equal another Some.
      private: template<typename T>
               using none =
                 typename std::enable_if<!std::is_same<Some, T>::value>::type;

      //// \brief Default constructor
      public: Some()
              {
              }

      /// \brief Destructor
      public: virtual ~Some()
              {
                delete this->value;
              }

      /// \brief Move constructor (destructive)
      /// \param[in] _s Some value to move
      public: Some(Some &&_s) : value(_s.value)
              {
                _s.value = nullptr;
              }

      /// \brief Copy constructor
      /// \param[in] _s Some value to copy
      public: Some(Some const &_s) : value(_s.value->Copy())
              {
              }

      /// \brief Set the contents to a different variable type.
      /// \param[i] _x Value to set this Some object to.
      public: template<typename T, typename U = decay<T>, typename = none<U>>
              Some(T &&_x) : value(new Data<U>(std::forward<T>(_x)))
              {
              }

      /// \brief Operator=
      /// \param[in] _s Some value to copy
      /// \return A reference to this.
      public: Some &operator=(Some _s)
              {
                this->Swap(*this, _s);
                return *this;
              }

      /// \brief Clear the value
      public: void Clear()
              {
                delete this->value;
                this->value = nullptr;
              }

      /// \brief Check if this Some is empty.
      /// \return True if this Some has not been set.
      public: bool Empty() const
              {
                return this->value;
              }

      /// \brief Check if this Some contains a specific type
      /// \return True if thie Some has the specified type.
      public: template<typename U>
              bool Is() const
              {
                return this->value ? this->value->Is(TypeId<U>()) : false;
              }

      /// \brief Set from a string.
      /// \param[in] _str A string containing a value
      /// \return True if this Some was set successfully.
      public: bool Set(const std::string &_str) const
              {
                if (this->value)
                  return this->value->Set(_str);
                else
                  return false;
              }

      /// \brief Get the value as a string.
      /// \return String version of the contents of this Some. An empty
      /// string is returned if this Some has not been set.
      public: std::string String() const
              {
                return this->value ? this->value->String() : "";
              }

      /// \brief Stream output operator
      /// \param[in] _out OStream reference
      /// \param[in] _s Some to output
      /// \return Reference to the ostream.
      public: friend std::ostream &operator<<(
                  std::ostream &_out, const Some &_s)
              {
                _out << _s.value ? _s.value->String() : "";
                return _out;
              }

      /// \brief A helper function to swap two Some values.
      /// \param[in,out] _s The Some variable that receives the second
      /// parameter.
      /// \param[in,out] _s The Some variable that receives the first
      /// parameter.
      private: static void Swap(Some &_s, Some &_r)
              {
                std::swap(_s.value, _r.value);
              }

      /// \brief This structure is used for RTTI.
      private: template<typename T>
               struct Type
               {
                 static void Id() {}
               };

      /// \brief Return the type id of the given type.
      /// \return RTTI of the given type.
      private: template<typename T>
               static id TypeId()
               {
                 return reinterpret_cast<id>(&Type<T>::Id);
               }

      /// \brief A base class for the value held by Some.
      private: struct Base
      {
        /// \brief Destructor.
        virtual ~Base() {}

        /// \brief Return true if the value is the specified type.
        /// \param[in] _type Type to check.
        /// \return True if the types match.
        virtual bool Is(id _type) const = 0;

        /// \brief Copy this object into a new Base.
        /// \return Pointer to a the new Base.
        virtual Base *Copy() const = 0;

        /// \brief Get this value as a string.
        /// \return String value
        virtual std::string String() const = 0;

        /// \brief Set the value from a string.
        /// \param[in] _str String to set the value from.
        /// \return True on success.
        virtual bool Set(const std::string &_str) = 0;
      } *value = nullptr;

      /// \brief Implementatio of Base.
      private: template<typename T>
               struct Data : Base, std::tuple<T>
      {
        using std::tuple<T>::tuple;

        /// \brief Helper function to get a reference to the
        /// contained value.
        /// \return A reference to the contained value.
        T &Get() &
        {
          return std::get<0>(*this);
        }

        /// \brief Helper function to get a const reference to the
        /// contained value.
        /// \return A const reference to the contained value.
        T const &Get() const &
        {
          return std::get<0>(*this);
        }

        /// \brief Return true if the value is the specified type.
        /// \param[in] _type Type to check.
        /// \return True if the types match.
        bool Is(id _type) const override
        {
          return _type == TypeId<T>();
        }

        /// \brief Copy this object into a new Base.
        /// \return Pointer to a the new Base.
        Base *Copy() const override
        {
          return new Data(this->Get());
        }

        /// \brief Get this value as a string.
        /// \return String value
        std::string String() const override
        {
          return sdf::lexicalCast<std::string>(this->Get());
        }

        /// \brief Set the value from a string.
        /// \param[in] _str String to set the value from.
        /// \return True on success.
        bool Set(const std::string &_str) override
        {
          try
          {
            this->Get() = sdf::lexicalCast<T>(_str);
          }
          // Catch invalid argument exception from std::stoi/stoul/stod/stof
          catch(std::invalid_argument &)
          {
            sdferr << "Invalid argument. Unable to set value ["
              << _str << "].\n";
            return false;
          }
          // Catch out of range exception from std::stoi/stoul/stod/stof
          catch(std::out_of_range &)
          {
            sdferr << "Out of range. Unable to set value ["
              << _str << " ].\n";
            return false;
          }
          catch(...)
          {
            sdferr << "Unknowne error. Unable to set value ["
              << _str << " ].\n";
            return false;
          }

          return true;
        }
      };

      /// \brief Static cast value to a type.
      /// \return A reference to the value.
      public: template<typename T>
      T &StaticValue()
      {
        return static_cast<Data<T>&>(*this->value).Get();
      }

      /// \brief Static cast value to a type.
      /// \return A const reference to the value.
      public: template<typename T>
      T const &StaticValue() const
      {
        return static_cast<Data<T> const&>(*this->value).Get();
      }

      /// \brief Dynamic cast value to a type.
      /// \return A reference to the value.
      public: template<typename T>
      T &DynamicValue()
      {
        return dynamic_cast<Data<T>&>(*this->value).Get();
      }

      /// \brief Dynamic cast value to a type.
      /// \return A const reference to the value.
      public: template<typename T>
      T const &DynamicValue() const
      {
        return dynamic_cast<Data<T> const&>(*this->value).Get();
      }
    };

    /// \brief Update function pointer.
    private: std::function<Some ()> updateFunc;

    /// \brief Private data
    private: ParamPrivate *dataPtr;

    /// \brief The value contained by this Param
    private: Some value;

    /// \brief The default value contained by this Param
    private: Some defaultValue;
  };

  /// \internal
  /// \brief Private data for the param class
  class ParamPrivate
  {
    /// \brief Key value
    public: std::string key;

    /// \brief True if the parameter is required.
    public: bool required;

    /// \brief True if the parameter is set.
    public: bool set;

    //// \brief Name of the type.
    public: std::string typeName;

    /// \brief Description of the parameter.
    public: std::string description;
  };


  ///////////////////////////////////////////////
  template<typename T>
  void Param::SetUpdateFunc(T _updateFunc)
  {
    this->updateFunc = _updateFunc;
  }

  ///////////////////////////////////////////////
  template<typename T>
  bool Param::Set(const T &_value)
  {
    try
    {
      this->SetFromString(sdf::lexicalCast<std::string>(_value));
    }
    catch(...)
    {
      sdferr << "Unable to set parameter["
             << this->dataPtr->key << "]."
             << "Type type used must have a stream input and output"
             << "operator, which allow sdf::lexicalCast to"
             << "function properly.\n";
      return false;
    }
    return true;
  }

  ///////////////////////////////////////////////
  template<typename T>
  bool Param::Get(T &_value) const
  {
    try
    {
      if (typeid(T) == typeid(bool) &&
          this->dataPtr->typeName == "string")
      {
        std::string strValue = this->value.StaticValue<std::string>();
        if (strValue == "true" || strValue  == "1")
          _value = sdf::lexicalCast<T>("1");
        else
          _value = sdf::lexicalCast<T>("0");
      }
      else
      {
        // TODO: Check the type here!!!
        _value = this->value.StaticValue<T>();
      }
    }
    catch(...)
    {
      sdferr << "Unable to convert parameter["
             << this->dataPtr->key << "] "
             << "whose type is["
             << this->dataPtr->typeName << "], to "
             << "type[" << typeid(T).name() << "]\n";
      return false;
    }
    return true;
  }

  ///////////////////////////////////////////////
  template<typename T>
  bool Param::GetDefault(T &_value) const
  {
    try
    {
      /// TODO: Check the type value!!!
      _value = this->defaultValue.StaticValue<T>();
    }
    catch(...)
    {
      sdferr << "Unable to convert parameter["
             << this->dataPtr->key << "] "
             << "whose type is["
             << this->dataPtr->typeName << "], to "
             << "type[" << typeid(T).name() << "]\n";
      return false;
    }
    return true;
  }

  ///////////////////////////////////////////////
  template<typename T>
  void Param::Init(const std::string &_value)
  {
    if (this->dataPtr->typeName == "bool")
    {
      std::string strValue = _value;
      std::transform(strValue.begin(), strValue.end(),
          strValue.begin(), ::tolower);

      if (strValue == "true" || strValue == "1")
        this->value = true;
      else if (strValue == "false" || strValue == "0")
        this->value = false;
      else
      {
        sdferr << "Unable to init bool parameter value from string["
          << _value << "]\n";
      }
    }
    else
    {
      try
      {
        this->value = sdf::lexicalCast<T>(_value);
      }
      catch(...)
      {
        sdferr << "Unable to init bool parameter value from string["
          << _value << "]\n";
      }
    }

    this->defaultValue = this->value;
    this->dataPtr->set = false;
  }

  ///////////////////////////////////////////////
  template<typename T>
  bool Param::IsType() const
  {
    return this->value.Is<T>();
  }
}
#endif
