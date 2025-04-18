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

#ifndef _NLU_ENGINE_DIALOGFLOW_H_
#define _NLU_ENGINE_DIALOGFLOW_H_

/****************************************************************************/
/** **/
/** CLASS INCLUDED **/
/** **/
/****************************************************************************/
#include "log.h"
#include "nlu_dialogflow_config.h"
#include "nlu_engine_interface.h"

#include "google/cloud/dialogflow_es/agents_client.h"
#include "google/cloud/dialogflow_es/contexts_client.h"
#include "google/cloud/dialogflow_es/sessions_client.h"

/****************************************************************************/
/** **/
/** DEFINITIONS AND MACROS**/
/** **/
/****************************************************************************/

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
    /** INTERFACE INLUEngine **/
    /** **/
    /****************************************************************************/

    class CNLUEngineDialogflow : public INLUEngine
    {
    public:
      explicit CNLUEngineDialogflow();

      ~CNLUEngineDialogflow();
      void initialize(INLUConfigImpl* a_config) override;
      std::unique_ptr<data::CNLUResult> process_input(
          const data::CInputSentence& a_input_sentence,
          const data::CNLUContext& a_input_context) override;

    private:
      CNLUDialogflowConfig* m_ptr_config = nullptr;
      static common::log::Logger& logger();

      std::string m_project_id;
      std::string m_session_id;

      std::unique_ptr<google::cloud::dialogflow_es::AgentsClient> m_ptr_dialogflow_client;
      std::unique_ptr<google::cloud::dialogflow_es::SessionsClient> m_ptr_dialogflow_session;
      std::unique_ptr<google::cloud::dialogflow_es::ContextsClient> m_ptr_dialogflow_context;

      void remove_contexts();
      void print_contexts() const;
      void set_context(const data::CNLUContext& a_nlu_input_context);
    };
  }  // namespace nlu

}  // namespace text_module

#endif  // _NLU_ENGINE_DIALOGFLOW_H_

/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/
