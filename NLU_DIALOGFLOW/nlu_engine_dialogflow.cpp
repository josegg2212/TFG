/*
* 4i Intelligent Insights CONFIDENTIAL
* Copyright 2022, 4i Intelligent Insights All Rights Reserved.

* The source code, information and material ("Material") contained herein is owned
* by 4i Intelligent Insights or its suppliers or licensors, and title to such Material
* remains with 4i Intelligent Insights or its suppliers or licensors. The Material contains
* proprietary information of 4i Intelligent Insights or its suppliers and licensors. The Material is
* protected by worldwide copyright laws and treaty provisions. No part of the Material
* may be used, copied, reproduced, modified, published, uploaded, posted, transmitted,
* distributed or disclosed in any way without 4i Intelligent Insights prior express written permission.
* No license under any patent, copyright or other intellectual property rights in the
* Material is granted to or conferred upon you, either expressly, by implication, inducement,
* estoppel or otherwise. Any license under such intellectual property rights must be express
* and approved by 4i Intelligent Insights in writing.

* Unless otherwise agreed by 4i Intelligent Insights in writing, you may not remove or alter this notice or any
* other notice embedded in Materials by 4i Intelligent Insights  or 4i Intelligent Insights suppliers or licensors in
* any way.
*/



/****************************************************************************/
/** **/
/** CLASS USED **/
/** **/
/****************************************************************************/
#include "nlu_engine_dialogflow.h"
#include <cstdlib>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/filesystem.hpp> 

using namespace std;
namespace dialogflow_es = ::google::cloud::dialogflow_es;

namespace
{
  common::log::Logger m_logger;
  std::once_flag initflag;
}  // namespace

/****************************************************************************/
/** **/
/** DEFINITIONS AND MACROS**/
/** **/
/****************************************************************************/

#define LIFE_SPAN_CONTEXT 2
#ifdef _WIN32
#define SET_ENV(name, value) _putenv_s(name, value)
#else
#define SET_ENV(name, value) setenv(name, value, 1)
#endif

/****************************************************************************/
/** **/
/** TYPEDEFS AND STRUCTURES **/
/** **/
/****************************************************************************/

namespace text_module
{
  namespace nlu
  {
    /****************************************************************************/
    /** **/
    /** PUBLIC **/
    /** **/
    /****************************************************************************/

    /**
     * Constructor.
     * @exception No exceptions.
     * @return
     */
    /*************************************************************************/

    CNLUEngineDialogflow::CNLUEngineDialogflow() 
    {
      LOG_BEGIN(logger());
      LOG_DEBUG(logger(), "NLU Dialogflow engine created");
      LOG_END(logger());
    }
      
    /**
     * Destructor.
     * @exception No exceptions.
     * @return
     */
    /*************************************************************************/      
	CNLUEngineDialogflow::~CNLUEngineDialogflow()
	{
      LOG_BEGIN(logger());
      m_ptr_dialogflow_context.release();
      m_ptr_dialogflow_session.release();
      m_ptr_dialogflow_client.release();
      LOG_END(logger());
    }

    void CNLUEngineDialogflow::initialize(INLUConfigImpl* a_config)
	{
      LOG_BEGIN(logger());
      LOG_DEBUG(logger(), "Initializing NLU Dialogflow engine");
      m_ptr_config = dynamic_cast<CNLUDialogflowConfig*>(a_config);
      m_project_id = m_ptr_config->get_project_id();

      if (boost::filesystem::exists(m_ptr_config->get_google_cloud_credentials().m_google_application_credentials) &&
          boost::filesystem::exists(m_ptr_config->get_google_cloud_credentials().m_grpc_default_ssl_roots_file_path))
      {
        LOG_DEBUG(logger(), "Setting google cloud credentials");
        std::string google_app_credentials =
            m_ptr_config->get_google_cloud_credentials().m_google_application_credentials;
        SET_ENV("GOOGLE_APPLICATION_CREDENTIALS", google_app_credentials.c_str());
        std::string grpc_default_ssl_roots_file =
            m_ptr_config->get_google_cloud_credentials().m_grpc_default_ssl_roots_file_path;
        SET_ENV("GRPC_DEFAULT_SSL_ROOTS_FILE_PATH", grpc_default_ssl_roots_file.c_str());
      }
      else
      {
        LOG_ERROR(
            logger(),
            "Credential file path of GOOGLE_APPLICATION_CREDENTIALS or GRPC_DEFAULT_SSL_ROOTS_FILE_PATH does not "
            "exist");
		  throw std::runtime_error("Throwing exception: Credential file path of GOOGLE_APPLICATION_CREDENTIALS or "
                                 "GRPC_DEFAULT_SSL_ROOTS_FILE_PATH does not exist"); 
      }

      m_ptr_dialogflow_client.reset(new dialogflow_es::AgentsClient(dialogflow_es::MakeAgentsConnection()));
      m_ptr_dialogflow_session.reset(new dialogflow_es::SessionsClient(dialogflow_es::MakeSessionsConnection()));
      m_ptr_dialogflow_context.reset(new dialogflow_es::ContextsClient(dialogflow_es::MakeContextsConnection()));

      boost::uuids::random_generator generator;
      auto session_uuid = generator();
      m_session_id = "projects/" + m_project_id + "/agent/sessions/" + boost::uuids::to_string(session_uuid);
      LOG_END(logger());
    }

    std::unique_ptr<data::CNLUResult> CNLUEngineDialogflow::process_input(
        const data::CInputSentence& a_input_sentence,
        const data::CNLUContext& a_input_context)
	{
      LOG_BEGIN(logger());
      std::unique_ptr<data::CNLUResult> ptr_result;

      google::cloud::dialogflow::v2::TextInput text;
      google::cloud::dialogflow::v2::QueryInput query_input;

      text.set_text(a_input_sentence.get_sentence());
      text.set_language_code(a_input_sentence.get_language());
      *query_input.mutable_text() = text;

      remove_contexts();
      set_context(a_input_context);

      
	  LOG_INFO(
          logger(), "Consuming sentence: " << a_input_sentence.get_uuid() << " : " << a_input_sentence.get_sentence());
      auto dialogflow_result = m_ptr_dialogflow_session->DetectIntent(m_session_id, query_input);
      if (dialogflow_result.ok())
      {
        unordered_map<string, string> entities;

        for (const auto& entity : dialogflow_result->query_result().parameters().fields())
        {
          string value;
          if (entity.second.has_null_value())
          {
            value = "";
          }
          else if (entity.second.has_string_value())
          {
            value = entity.second.string_value();
          }
          else if (entity.second.has_number_value())
          {
            value = to_string(entity.second.number_value());
          }
          else if (entity.second.has_bool_value())
          {
            value = to_string(entity.second.bool_value());
          }
          else
          {
            LOG_WARNING(logger(), "Entity value is a list or a struct");
          }
          entities[entity.first] = value;
        }

        ptr_result.reset(new data::CNLUResult(
            a_input_sentence.get_uuid(),
            a_input_sentence.get_sentence(),
            dialogflow_result->query_result().intent().display_name(),
            entities));
      }
      else
      {
        LOG_ERROR(logger(), "DialogFlow result is not ok " << dialogflow_result.status());
      }

      LOG_END(logger());
      return ptr_result;
	}

	 /**
     * Remove contexts
     * @exception No exceptions.
     * @return
     */
    /*************************************************************************/
    void CNLUEngineDialogflow::remove_contexts()
    {
      LOG_BEGIN(logger());
      google::cloud::dialogflow::v2::DeleteAllContextsRequest delete_all_contexts_request;
      delete_all_contexts_request.set_parent(m_session_id);
      m_ptr_dialogflow_context->DeleteAllContexts(delete_all_contexts_request);
      LOG_END(logger());
    }

    /**
     * Print contexts
     * @exception No exceptions.
     * @return
     */
    /*************************************************************************/
    void CNLUEngineDialogflow::print_contexts() const
    {
      LOG_BEGIN(logger());
      LOG_DEBUG(logger(), "The current contexts are: ");
      for (auto& context : m_ptr_dialogflow_context->ListContexts(m_session_id))
      {
        LOG_DEBUG(logger(), context.value().name());
      }
      LOG_END(logger());
    }

	 /**
     * Set input context
     * @param a_nlu_input_context input context for given input sentence
     * @exception No exceptions.
     * @return
     */
    /*************************************************************************/
    void CNLUEngineDialogflow::set_context(const data::CNLUContext& a_nlu_input_context)
    {
      LOG_BEGIN(logger());
      for (auto& context_name : a_nlu_input_context.get_context_names())
      {
        LOG_DEBUG(logger(), "Setting context: " << context_name);
        google::cloud::dialogflow::v2::CreateContextRequest context_request;
        google::cloud::dialogflow::v2::Context context;

        std::string context_path_name = m_session_id + "/contexts/" + context_name;
        context.set_name(context_path_name);
        context.set_lifespan_count(LIFE_SPAN_CONTEXT);

        *context_request.mutable_context() = context;
        context_request.set_parent(m_session_id);

        auto context_status = m_ptr_dialogflow_context->CreateContext(context_request);
        LOG_DEBUG(logger(), "The status is: " << context_status.status());
      }
      print_contexts();
      LOG_END(logger());
    }

	/**
     * STATIC Gets the logger associated to the class, using "Construct On First Use Idiom".
     * @return logger.
     */
    /*************************************************************************/
    common::log::Logger& CNLUEngineDialogflow::logger()
    {
      std::call_once(initflag, []() {
        m_logger = common::log::get_logger("text_module::nlu_dialogflow::CNLUDialogflowWorkingThread");
      });
      return m_logger;
    }

  }
}

/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/
