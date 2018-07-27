/*
 * lateral_prov_main.c
 *
 *  Created on: July 27, 2018
 *      Author: Xing
 */

#include "configuration/option.h"
#include "instrumentation/timing_instrumentation.h"
#include "provenance_rewriter/lateral_rewrites/lateral_prov_main.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/list/list.h"


