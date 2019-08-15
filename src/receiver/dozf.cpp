#include "dozf.hpp"

using namespace arma;
DoZF::DoZF(int in_tid, int in_zf_block_size, int in_transpose_block_size,
    moodycamel::ConcurrentQueue<Event_data> *in_complete_task_queue, moodycamel::ProducerToken *in_task_ptok,
    complex_float **in_csi_buffer, complex_float **in_precoder_buffer, complex_float **in_pred_csi_buffer, 
    double **in_ZF_task_duration, int *in_ZF_task_count) 
{
    tid = in_tid;
    zf_block_size = in_zf_block_size;
    transpose_block_size = in_transpose_block_size;
    complete_task_queue_ = in_complete_task_queue;
    task_ptok = in_task_ptok;

    csi_buffer_ = in_csi_buffer;
    precoder_buffer_ = in_precoder_buffer;
    pred_csi_buffer_ = in_pred_csi_buffer;


    ZF_task_duration = in_ZF_task_duration;
    ZF_task_count = in_ZF_task_count;

    csi_gather_buffer = (complex_float *)aligned_alloc(BS_ANT_NUM * UE_NUM * sizeof(complex_float), BS_ANT_NUM * UE_NUM * sizeof(complex_float));

}


DoZF::~DoZF()
{
    free(csi_gather_buffer);   
}


void DoZF::ZF(int offset)
{
// #if DEBUG_UPDATE_STATS
//     double start_time = get_time();
// #endif 
    int frame_id, sc_id;
    interpreteOffset2d(OFDM_DATA_NUM, offset, &frame_id, &sc_id);

#if DEBUG_PRINT_IN_TASK
        printf("In doZF thread %d: frame: %d, subcarrier: %d\n", tid, frame_id, sc_id);
#endif
    int max_sc_ite;
    if (sc_id + zf_block_size <= OFDM_DATA_NUM) 
        max_sc_ite = zf_block_size;
    else
        max_sc_ite = OFDM_DATA_NUM - sc_id;
    // i = 0, 1, ..., 31
    for (int i = 0; i < max_sc_ite; i++) {

#if DEBUG_UPDATE_STATS
        double start_time1 = get_time();
#endif 
        int cur_sc_id = sc_id + i;
        int cur_offset = offset + i;
        // // directly gather data from FFT buffer
        // __m256i index = _mm256_setr_epi32(0, 1, OFDM_CA_NUM * 2, OFDM_CA_NUM * 2 + 1, OFDM_CA_NUM * 4, OFDM_CA_NUM * 4 + 1, OFDM_CA_NUM * 6, OFDM_CA_NUM * 6 + 1);
        __m256i index = _mm256_setr_epi32(0, 1, transpose_block_size * 2, transpose_block_size * 2 + 1, transpose_block_size * 4, transpose_block_size * 4 + 1, transpose_block_size * 6, transpose_block_size * 6 + 1);
        // __m256i index = _mm256_setr_epi64x(0, transpose_block_size/2, transpose_block_size/2 * 2, transpose_block_size/2 * 3);

        int transpose_block_id = cur_sc_id / transpose_block_size;
        int sc_inblock_idx = cur_sc_id % transpose_block_size;
        int offset_in_csi_buffer = transpose_block_id * BS_ANT_NUM * transpose_block_size  + sc_inblock_idx;
        // double *tar_csi_ptr = (double *)csi_gather_buffer[tid];
        int subframe_offset = frame_id * UE_NUM;
        // int subframe_offset = frame_id * subframe_num_perframe;
        float *tar_csi_ptr = (float *)csi_gather_buffer;

        // // if (sc_id == 4) {
        // //     cout<<"csi_buffer_ for subframe "<<subframe_offset<<endl;
        // //     for (int i=0;i<BS_ANT_NUM*OFDM_CA_NUM; i++) {
        // //         cout<<"("<<i<<",  "<<csi_buffer_.CSI[subframe_offset][i].real<<","<<csi_buffer_.CSI[subframe_offset][i].imag<<") ";
        // //     }

        // //     cout<<endl;
        // // }

        // gather data for all users and antennas
        // printf("In doZF thread %d: frame: %d, subcarrier: %d\n", tid, frame_id, sc_id);
        int mat_elem = UE_NUM * BS_ANT_NUM;
        int cache_line_num = mat_elem / 8;
        // for (int line_idx = 0; line_idx < cache_line_num; line_idx ++) {
        //     _mm_prefetch((char *)precoder_buffer_.precoder[cur_offset + 8 * line_idx], _MM_HINT_ET1);
        //     // _mm_prefetch((char *)(tar_csi_ptr + 16 * line_idx), _MM_HINT_ET1);
        // }

        for (int ue_idx = 0; ue_idx < UE_NUM; ue_idx++) {
            float *src_csi_ptr = (float *)csi_buffer_[subframe_offset + ue_idx] + offset_in_csi_buffer * 2;
            for (int ant_idx = 0; ant_idx < BS_ANT_NUM; ant_idx += 4) {
                // fetch 4 complex floats for 4 ants
                __m256 t_csi = _mm256_i32gather_ps(src_csi_ptr, index, 4);
                _mm256_store_ps(tar_csi_ptr, t_csi);
                // printf("UE %d, ant %d, data: %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n", ue_idx, ant_idx, *((float *)tar_csi_ptr), *((float *)tar_csi_ptr+1), 
                //         *((float *)tar_csi_ptr+2), *((float *)tar_csi_ptr+3),  *((float *)tar_csi_ptr+4), *((float *)tar_csi_ptr+5));
                src_csi_ptr += 8 * transpose_block_size;
                tar_csi_ptr += 8;
            }
        }
   

        // // gather data for all users and antennas
        // for (int ue_idx = 0; ue_idx < UE_NUM; ue_idx++) {
        //     float *src_csi_ptr = (float *)fft_buffer_.FFT_outputs[subframe_offset+ue_idx] + sc_id * 2;
        //     for (int ant_idx = 0; ant_idx < BS_ANT_NUM; ant_idx += 4) {
        //         // fetch 4 complex floats for 4 ants
        //         __m256 pilot_rx = _mm256_i32gather_ps(src_csi_ptr, index, 4);
               
        //         if (pilots_[sc_id] > 0) {
        //             _mm256_store_ps(tar_csi_ptr, pilot_rx);
        //         }     
        //         else if (pilots_[sc_id] < 0){
        //             __m256 pilot_tx = _mm256_set1_ps(pilots_[sc_id]);
        //             __m256 csi_est = _mm256_mul_ps(pilot_rx, pilot_tx);
        //             _mm256_store_ps(tar_csi_ptr, csi_est);
        //         }
        //         else {
        //             _mm256_store_ps(tar_csi_ptr, _mm256_setzero_ps());
        //         }

        //         // printf("Frame %d, sc: %d, UE %d, ant %d, data: %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n", frame_id, sc_id, ue_idx, ant_idx, *((float *)tar_csi_ptr), *((float *)tar_csi_ptr+1), 
        //         //         *((float *)tar_csi_ptr+2), *((float *)tar_csi_ptr+3),  *((float *)tar_csi_ptr+4), *((float *)tar_csi_ptr+5));
        //         src_csi_ptr += 8 * OFDM_CA_NUM;
        //         tar_csi_ptr += 8;

        //     }
        // }

    #if DEBUG_UPDATE_STATS_DETAILED
        double duration1 = get_time() - start_time1;
        ZF_task_duration[tid * 8][1] += duration1;
        // double start_time2 = get_time();
    #endif
        cx_float *ptr_in = (cx_float *)csi_gather_buffer;
        // cx_float *ptr_in = (cx_float *)csi_buffer_.CSI[offset];
        cx_fmat mat_input(ptr_in, BS_ANT_NUM, UE_NUM, false);
        // cout<<"CSI matrix"<<endl;
        // cout<<mat_input.st()<<endl;
        // cx_fmat mat_input(ptr_in, UE_NUM, BS_ANT_NUM, false);
        // cx_float *ptr_out = (cx_float *)precoder_buffer_temp[tid];
        cx_float *ptr_out = (cx_float *)precoder_buffer_[cur_offset];
        cx_fmat mat_output(ptr_out, UE_NUM, BS_ANT_NUM, false);
        // cx_fmat mat_output(ptr_out, BS_ANT_NUM, UE_NUM, false);

    #if DEBUG_UPDATE_STATS_DETAILED
        double start_time2 = get_time();
        double duration2 = start_time2 - start_time1;
        ZF_task_duration[tid * 8][2] += duration2;
        // double start_time2 = get_time();
    #endif
   
        
        pinv(mat_output, mat_input, 1e-1, "dc");

        // cout<<"Precoder:" <<mat_output<<endl;
    #if DEBUG_UPDATE_STATS_DETAILED    
        double duration3 = get_time() - start_time2;
        ZF_task_duration[tid * 8][3] += duration3;
        // double start_time3 = get_time();
    #endif
        
    
        // float *tar_ptr = (float *)precoder_buffer_.precoder[cur_offset];
        // // float temp = *tar_ptr;
        // float *src_ptr = (float *)ptr_out;

        // // int mat_elem = UE_NUM * BS_ANT_NUM;
        // // int cache_line_num = mat_elem / 8;
        // for (int line_idx = 0; line_idx < cache_line_num; line_idx++) {
        //     _mm256_stream_ps(tar_ptr, _mm256_load_ps(src_ptr));
        //     _mm256_stream_ps(tar_ptr + 8, _mm256_load_ps(src_ptr + 8));
        //     tar_ptr += 16;
        //     src_ptr += 16;
        // }
    // #if DEBUG_UPDATE_STATS    
    //     double duration3 = get_time() - start_time3;
    //     ZF_task_duration[tid][3] += duration3;
    // #endif
    #if DEBUG_UPDATE_STATS   
        ZF_task_count[tid * 16] = ZF_task_count[tid * 16]+1; 
        double duration = get_time() - start_time1;
        ZF_task_duration[tid * 8][0] += duration;
        if (duration > 500) {
            printf("Thread %d ZF takes %.2f\n", tid, duration);
        }
    #endif

    }



    // inform main thread
    Event_data ZF_finish_event;
    ZF_finish_event.event_type = EVENT_ZF;
    ZF_finish_event.data = offset;
    

    if ( !complete_task_queue_->enqueue(*task_ptok, ZF_finish_event ) ) {
        printf("ZF message enqueue failed\n");
        exit(0);
    }


}


void DoZF::Predict(int offset) 
{
    int frame_id, sc_id;
    interpreteOffset2d(OFDM_DATA_NUM, offset, &frame_id, &sc_id);
    int offset_next_frame = ((frame_id+1)%TASK_BUFFER_FRAME_NUM)*OFDM_CA_NUM+sc_id;
    // Use stale CSI as predicted CSI
    // TODO: add prediction algorithm
    cx_float *ptr_in = (cx_float *)pred_csi_buffer_[sc_id];
    memcpy(ptr_in, (cx_float *)csi_buffer_[offset], sizeof(cx_float)*BS_ANT_NUM*UE_NUM);
    cx_fmat mat_input(ptr_in, BS_ANT_NUM, UE_NUM, false);
    cx_float *ptr_out = (cx_float *)precoder_buffer_[offset_next_frame];
    cx_fmat mat_output(ptr_out, UE_NUM, BS_ANT_NUM, false);
    pinv(mat_output, mat_input, 1e-1, "dc");

    // inform main thread
    Event_data pred_finish_event;
    pred_finish_event.event_type = EVENT_ZF;
    pred_finish_event.data = offset_next_frame;

    if ( !complete_task_queue_->enqueue(*task_ptok, pred_finish_event ) ) {
        printf("Prediction message enqueue failed\n");
        exit(0);
    }
}