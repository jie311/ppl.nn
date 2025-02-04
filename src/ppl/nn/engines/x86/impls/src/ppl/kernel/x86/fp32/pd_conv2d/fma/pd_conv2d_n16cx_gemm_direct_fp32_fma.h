// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#ifndef __ST_PPL_KERNEL_X86_FP32_PD_CONV2D_FMA_PD_CONV2D_N16CX_GEMM_DIRECT_FP32_FMA_H_
#define __ST_PPL_KERNEL_X86_FP32_PD_CONV2D_FMA_PD_CONV2D_N16CX_GEMM_DIRECT_FP32_FMA_H_

#include "ppl/kernel/x86/fp32/pd_conv2d.h"
#include "ppl/kernel/x86/common/internal_include.h"

namespace ppl { namespace kernel { namespace x86 {

/*

    Key Point of Post-Depthwise Conv:
        Avoiding L3 data transfer.
        Origin data path:    L3 -> Conv -> L3 -> DW Conv -> L3
        Optimized data path: L3 -> Conv -> L2 -> DW Conv -> L3

*/

class pd_conv2d_n16cx_gemm_direct_fp32_fma_executor final : public pd_conv2d_fp32_executor {
public:
    pd_conv2d_n16cx_gemm_direct_fp32_fma_executor(conv2d_fp32_executor *exec, conv2d_fp32_executor *depthwise_exec)
        : pd_conv2d_fp32_executor(exec, depthwise_exec) {}

    uint64_t cal_temp_buffer_size() override;
    ppl::common::RetCode prepare() override;
    ppl::common::RetCode execute() override;

private:
    struct kernel_schedule_param {
        // Preprocessed param
        int64_t ic_per_grp;
        int64_t oc_per_grp;
        int64_t padded_ic;
        int64_t padded_oc;

        // Kernel tunning
        int64_t gd_ker_blk;
        int64_t dw_ker_blk;
        int64_t oh_l2_blk;
        int64_t ic_l2_blk;
        int64_t ic_l2_cnt;
        int64_t oc_l2_blk;
        int64_t mb_l3_blk;
        int64_t grp_l3_blk;
        int32_t use_nt_store;

        int64_t mode;
        uint64_t gd_temp_buffer_size;
        uint64_t dw_temp_buffer_size;
    } schedule_param_;

    ppl::nn::TensorShape inter_shape_;

    void init_preproc_param();
    void cal_kernel_tunning_param();
    ppl::common::RetCode fuse_execute();
    ppl::common::RetCode separate_execute();

    static int64_t cal_ic_l2_blk(const conv2d_fp32_param &param);
};

class pd_conv2d_n16cx_gemm_direct_fp32_fma_manager final : public pd_conv2d_fp32_manager {
public:
    pd_conv2d_n16cx_gemm_direct_fp32_fma_manager() {}
    pd_conv2d_n16cx_gemm_direct_fp32_fma_manager(conv2d_fp32_manager *mgr, conv2d_fp32_manager *depthwise_mgr)
        : pd_conv2d_fp32_manager(mgr, depthwise_mgr) {}
    pd_conv2d_fp32_executor *gen_executor() override {
        return new pd_conv2d_n16cx_gemm_direct_fp32_fma_executor(conv2d_manager_->gen_executor(), depthwise_conv2d_manager_->gen_executor());
    }
};

}}}; // namespace ppl::kernel::x86

#endif