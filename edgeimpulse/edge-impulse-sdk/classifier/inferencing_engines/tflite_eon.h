/* The Clear BSD License
 *
 * Copyright (c) 2025 EdgeImpulse Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _EI_CLASSIFIER_INFERENCING_ENGINE_TFLITE_EON_H_
#define _EI_CLASSIFIER_INFERENCING_ENGINE_TFLITE_EON_H_

#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE) && (EI_CLASSIFIER_COMPILED == 1)

#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/classifier/ei_aligned_malloc.h"
#include "edge-impulse-sdk/classifier/ei_fill_result_struct.h"
#include "edge-impulse-sdk/classifier/ei_model_types.h"
#include "edge-impulse-sdk/classifier/inferencing_engines/tflite_helper.h"
#include "edge-impulse-sdk/classifier/ei_run_dsp.h"

/**
 * Setup the TFLite runtime
 *
 * @param      ctx_start_us       Pointer to the start time
 * @param      input              Pointer to input tensor
 * @param      output             Pointer to output tensor
 * @param      micro_tensor_arena Pointer to the arena that will be allocated
 *
 * @return  EI_IMPULSE_OK if successful
 */
static EI_IMPULSE_ERROR inference_tflite_setup(
    ei_learning_block_config_tflite_graph_t *block_config,
    uint64_t *ctx_start_us,
    TfLiteTensor* input,
    TfLiteTensor* output,
    TfLiteTensor* output_labels,
    TfLiteTensor* output_scores,
    ei_unique_ptr_t& p_tensor_arena) {

    ei_config_tflite_eon_graph_t *graph_config = (ei_config_tflite_eon_graph_t*)block_config->graph_config;

    *ctx_start_us = ei_read_timer_us();

    TfLiteStatus init_status = graph_config->model_init(ei_aligned_calloc);
    if (init_status != kTfLiteOk) {
        ei_printf("Failed to initialize the model (error code %d)\n", init_status);
        return EI_IMPULSE_TFLITE_ARENA_ALLOC_FAILED;
    }

    TfLiteStatus status;

    status = graph_config->model_input(0, input);
    if (status != kTfLiteOk) {
        return EI_IMPULSE_TFLITE_ERROR;
    }
    status = graph_config->model_output(block_config->output_data_tensor, output);
    if (status != kTfLiteOk) {
        return EI_IMPULSE_TFLITE_ERROR;
    }

    if (block_config->object_detection_last_layer == EI_CLASSIFIER_LAST_LAYER_SSD) {
        status = graph_config->model_output(block_config->output_score_tensor, output_scores);
        if (status != kTfLiteOk) {
            return EI_IMPULSE_TFLITE_ERROR;
        }
        status = graph_config->model_output(block_config->output_labels_tensor, output_labels);
        if (status != kTfLiteOk) {
            return EI_IMPULSE_TFLITE_ERROR;
        }
    }

    return EI_IMPULSE_OK;
}

/**
 * Run TFLite model
 *
 * @param   ctx_start_us    Start time of the setup function (see above)
 * @param   output          Output tensor
 * @param   interpreter     TFLite interpreter (non-compiled models)
 * @param   tensor_arena    Allocated arena (will be freed)
 * @param   result          Struct for results
 * @param   debug           Whether to print debug info
 *
 * @return  EI_IMPULSE_OK if successful
 */
static EI_IMPULSE_ERROR inference_tflite_run(
    const ei_impulse_t *impulse,
    ei_learning_block_config_tflite_graph_t *block_config,
    uint64_t ctx_start_us,
    TfLiteTensor* output,
    TfLiteTensor* labels_tensor,
    TfLiteTensor* scores_tensor,
    uint8_t* tensor_arena,
    ei_impulse_result_t *result,
    bool debug) {

    ei_config_tflite_eon_graph_t *graph_config = (ei_config_tflite_eon_graph_t*)block_config->graph_config;

    if (graph_config->model_invoke() != kTfLiteOk) {
        return EI_IMPULSE_TFLITE_ERROR;
    }

    uint64_t ctx_end_us = ei_read_timer_us();

    result->timing.classification_us = ctx_end_us - ctx_start_us;
    result->timing.classification = (int)(result->timing.classification_us / 1000);

    // Read the predicted y value from the model's output tensor
    if (debug) {
        ei_printf("Predictions (time: %d ms.):\n", result->timing.classification);
    }

    EI_IMPULSE_ERROR fill_res = fill_result_struct_from_output_tensor_tflite(
        impulse, block_config, output, labels_tensor, scores_tensor, result, debug);

    if (fill_res != EI_IMPULSE_OK) {
        return fill_res;
    }

    if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
        return EI_IMPULSE_CANCELED;
    }

    return EI_IMPULSE_OK;
}

/**
 * @brief      Do neural network inferencing over a signal (from the DSP)
 *
 * @param      fmatrix  Processed matrix
 * @param      result   Output classifier results
 * @param[in]  debug    Debug output enable
 *
 * @return     The ei impulse error.
 */
EI_IMPULSE_ERROR run_nn_inference_from_dsp(
    ei_learning_block_config_tflite_graph_t *block_config,
    signal_t *signal,
    matrix_t *output_matrix)
{
    TfLiteTensor input;
    TfLiteTensor output;
    TfLiteTensor output_scores;
    TfLiteTensor output_labels;
    uint64_t ctx_start_us = ei_read_timer_us();
    ei_unique_ptr_t p_tensor_arena(nullptr, ei_aligned_free);
    ei_config_tflite_eon_graph_t *graph_config = (ei_config_tflite_eon_graph_t*)block_config->graph_config;

    EI_IMPULSE_ERROR init_res = inference_tflite_setup(
        block_config,
        &ctx_start_us,
        &input,
        &output,
        &output_labels,
        &output_scores,
        p_tensor_arena);

    if (init_res != EI_IMPULSE_OK) {
        return init_res;
    }

    auto input_res = fill_input_tensor_from_signal(signal, &input);
    if (input_res != EI_IMPULSE_OK) {
        return input_res;
    }

    // invoke the model
    if (graph_config->model_invoke() != kTfLiteOk) {
        return EI_IMPULSE_TFLITE_ERROR;
    }

    auto output_res = fill_output_matrix_from_tensor(&output, output_matrix);
    if (output_res != EI_IMPULSE_OK) {
        return output_res;
    }

    if (graph_config->model_reset(ei_aligned_free) != kTfLiteOk) {
        return EI_IMPULSE_TFLITE_ERROR;
    }

    return EI_IMPULSE_OK;
}

/**
 * @brief      Do neural network inferencing over a feature matrix
 *
 * @param      fmatrix  Processed matrix
 * @param      result   Output classifier results
 * @param[in]  debug    Debug output enable
 *
 * @return     The ei impulse error.
 */
EI_IMPULSE_ERROR run_nn_inference(
    const ei_impulse_t *impulse,
    ei_feature_t *fmatrix,
    uint32_t learn_block_index,
    uint32_t* input_block_ids,
    uint32_t input_block_ids_size,
    ei_impulse_result_t *result,
    void *config_ptr,
    bool debug = false)
{
    ei_learning_block_config_tflite_graph_t *block_config = (ei_learning_block_config_tflite_graph_t*)config_ptr;
    ei_config_tflite_eon_graph_t *graph_config = (ei_config_tflite_eon_graph_t*)block_config->graph_config;

    TfLiteTensor input;
    TfLiteTensor output;
    TfLiteTensor output_scores;
    TfLiteTensor output_labels;

    uint64_t ctx_start_us = ei_read_timer_us();
    ei_unique_ptr_t p_tensor_arena(nullptr, ei_aligned_free);

    EI_IMPULSE_ERROR init_res = inference_tflite_setup(
        block_config,
        &ctx_start_us,
        &input,
        &output,
        &output_labels,
        &output_scores,
        p_tensor_arena);

    if (init_res != EI_IMPULSE_OK) {
        return init_res;
    }

    uint8_t* tensor_arena = static_cast<uint8_t*>(p_tensor_arena.get());

    size_t mtx_size = impulse->dsp_blocks_size + impulse->learning_blocks_size;
    auto input_res = fill_input_tensor_from_matrix(fmatrix, &input, input_block_ids, input_block_ids_size, mtx_size);
    if (input_res != EI_IMPULSE_OK) {
        return input_res;
    }

    EI_IMPULSE_ERROR run_res = inference_tflite_run(
        impulse,
        block_config,
        ctx_start_us,
        &output,
        &output_labels,
        &output_scores,
        tensor_arena, result, debug);

    if (result->copy_output) {
        auto output_res = fill_output_matrix_from_tensor(&output, fmatrix[impulse->dsp_blocks_size + learn_block_index].matrix);
        if (output_res != EI_IMPULSE_OK) {
            return output_res;
        }
    }

    graph_config->model_reset(ei_aligned_free);

    if (run_res != EI_IMPULSE_OK) {
        return run_res;
    }

    return EI_IMPULSE_OK;
}

#if EI_CLASSIFIER_QUANTIZATION_ENABLED == 1
/**
 * Special function to run the classifier on images, only works on TFLite models (either interpreter or EON or for tensaiflow)
 * that allocates a lot less memory by quantizing in place. This only works if 'can_run_classifier_image_quantized'
 * returns EI_IMPULSE_OK.
 */
EI_IMPULSE_ERROR run_nn_inference_image_quantized(
    const ei_impulse_t *impulse,
    signal_t *signal,
    ei_impulse_result_t *result,
    void *config_ptr,
    bool debug = false) {

    ei_learning_block_config_tflite_graph_t *block_config = (ei_learning_block_config_tflite_graph_t*)config_ptr;
    ei_config_tflite_eon_graph_t *graph_config = (ei_config_tflite_eon_graph_t*)block_config->graph_config;

    memset(result, 0, sizeof(ei_impulse_result_t));

    uint64_t ctx_start_us;
    TfLiteTensor input;
    TfLiteTensor output;
    TfLiteTensor output_scores;
    TfLiteTensor output_labels;

    ei_unique_ptr_t p_tensor_arena(nullptr, ei_aligned_free);

    EI_IMPULSE_ERROR init_res = inference_tflite_setup(
        block_config,
        &ctx_start_us,
        &input, &output,
        &output_labels,
        &output_scores,
        p_tensor_arena);

    if (init_res != EI_IMPULSE_OK) {
        return init_res;
    }

    if (input.type != TfLiteType::kTfLiteInt8 && input.type != TfLiteType::kTfLiteUInt8) {
        return EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES;
    }

    uint64_t dsp_start_us = ei_read_timer_us();

    // features matrix maps around the input tensor to not allocate any memory
    ei::matrix_i8_t features_matrix(1, impulse->nn_input_frame_size, input.data.int8);

    // run DSP process and quantize automatically
    int ret = extract_image_features_quantized(signal, &features_matrix, impulse->dsp_blocks[0].config, input.params.scale, input.params.zero_point,
        impulse->frequency, impulse->learning_blocks[0].image_scaling);

    if (ret != EIDSP_OK) {
        ei_printf("ERR: Failed to run DSP process (%d)\n", ret);
        return EI_IMPULSE_DSP_ERROR;
    }

    if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
        return EI_IMPULSE_CANCELED;
    }

    result->timing.dsp_us = ei_read_timer_us() - dsp_start_us;
    result->timing.dsp = (int)(result->timing.dsp_us / 1000);

    if (debug) {
        ei_printf("Features (%d ms.): ", result->timing.dsp);
        for (size_t ix = 0; ix < features_matrix.cols; ix++) {
            ei_printf_float((features_matrix.buffer[ix] - input.params.zero_point) * input.params.scale);
            ei_printf(" ");
        }
        ei_printf("\n");
    }

    ctx_start_us = ei_read_timer_us();

    EI_IMPULSE_ERROR run_res = inference_tflite_run(
        impulse,
        block_config,
        ctx_start_us,
        &output,
        &output_labels,
        &output_scores,
        static_cast<uint8_t*>(p_tensor_arena.get()),
        result,
        debug);

    graph_config->model_reset(ei_aligned_free);

    if (run_res != EI_IMPULSE_OK) {
        return run_res;
    }

    return EI_IMPULSE_OK;
}
#endif // EI_CLASSIFIER_QUANTIZATION_ENABLED == 1

__attribute__((unused)) int extract_tflite_eon_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float frequency) {
    ei_dsp_config_tflite_eon_t *dsp_config = (ei_dsp_config_tflite_eon_t*)config_ptr;

    ei_config_tflite_eon_graph_t ei_config_tflite_graph_0 = {
        .implementation_version = 1,
        .model_init = dsp_config->init_fn,
        .model_invoke = dsp_config->invoke_fn,
        .model_reset = dsp_config->reset_fn,
        .model_input = dsp_config->input_fn,
        .model_output = dsp_config->output_fn,
    };

    ei_learning_block_config_tflite_graph_t ei_learning_block_config = {
        .implementation_version = 1,
        .classification_mode = EI_CLASSIFIER_CLASSIFICATION_MODE_DSP,
        .block_id = dsp_config->block_id,
        .object_detection = false,
        .object_detection_last_layer = EI_CLASSIFIER_LAST_LAYER_UNKNOWN,
        .output_data_tensor = 0,
        .output_labels_tensor = 255,
        .output_score_tensor = 255,
        .threshold = 0,
        .quantized = 0,
        .compiled = 1,
        .graph_config = &ei_config_tflite_graph_0
    };

    auto x = run_nn_inference_from_dsp(&ei_learning_block_config, signal, output_matrix);
    if (x != 0) {
        return x;
    }

    return EIDSP_OK;
}

#endif // (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE) && (EI_CLASSIFIER_COMPILED == 1)
#endif // _EI_CLASSIFIER_INFERENCING_ENGINE_TFLITE_EON_H_
