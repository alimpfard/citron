
ctr_object* ctr_ccomp_get_stub(ctr_function_ptr_t func, ctr_object* receiver, ctr_argument* args) {
  switch (func) {
    case &ctr_nil_is_nil:
      return ctr_build_bool(1);

    case &ctr_nil_assign:
    case &ctr_block_error:
    case &ctr_console_brk:
      return CtrStdNil;

    case &ctr_object_to_string:
    case &ctr_bool_to_string:
    case &ctr_object_type:
    case &ctr_number_uint_binrep:
    case &ctr_number_to_string:
    case &ctr_string_fromto:
    case &ctr_string_from_length:
    case &ctr_string_concat:
    case &ctr_string_append:
    case &ctr_string_multiply:
    case &ctr_string_trim:
    case &ctr_string_ltrim:
    case &ctr_string_rtrim:
    case &ctr_string_padding_left:
    case &ctr_string_padding_right:
    case &ctr_string_html_escape:
    case &ctr_string_count_of:
    case &ctr_string_slice:
    case &ctr_string_at:
    case &ctr_string_replace_with:
    case &ctr_string_to_lower:
    case &ctr_string_to_upper:
    case &ctr_string_skip:
    case &ctr_string_to_lower1st:
    case &ctr_string_to_upper1st:
    case &ctr_string_find_pattern_do:
    case &ctr_string_find_pattern_options_do:
    case &ctr_string_to_string:
    case &ctr_string_format:
    case &ctr_string_format_map:
    case &ctr_string_quotes_escape:
    case &ctr_string_dquotes_escape:
    case &ctr_string_append_byte:
    case &ctr_string_csub:
    case &ctr_string_cadd:
    case &ctr_string_randomize_bytes:
    case &ctr_string_reverse:
    case &ctr_string_assign:
    case &ctr_nil_to_string:
    case &ctr_array_type:
    case &ctr_array_join:
    case &ctr_array_to_string:
    case &ctr_map_type:
    case &ctr_map_to_string:
      return CtrStdString;


    case &ctr_object_message:
    case &ctr_object_if_false:
    case &ctr_object_if_true:
    case &ctr_bool_if_true:
    case &ctr_bool_if_false:
    case &ctr_bool_either_or:
    case &ctr_number_respond_to:
    case &ctr_string_eval:
    case &ctr_block_runIt:
    case &ctr_block_run:
      int cc = CTR_CCOMP_SIMULATION;
      CTR_CCOMP_SIMULATION = 1; //enable simulation
      ctr_object* result = ctr_send_message(receiver, args);
      CTR_CCOMP_SIMULATION = cc; //set it back to its original value
      return result;

    case &ctr_object_remote:
    case &ctr_object_learn_meaning:
    case &ctr_object_respond_and:
    case &ctr_object_respond_and_and:
    case &ctr_object_myself:
    case &ctr_object_do:
    case &ctr_object_done:
    case &ctr_object_on_do:
    case &ctr_object_respond:
    case &ctr_object_make:
    case &ctr_object_ctor:
    case &ctr_object_assign:
    case &ctr_object_attr_accessor:
    case &ctr_object_attr_reader:
    case &ctr_object_attr_writer:
    case &ctr_object_make_hiding:
    case &ctr_object_swap:
    case &ctr_array_foldl:
    case &ctr_array_unshift:
    case &ctr_array_shift:
    case &ctr_array_count:
    case &ctr_array_pop:
    case &ctr_array_index:
    case &ctr_array_contains:
    case &ctr_array_get:
    case &ctr_array_head:
    case &ctr_array_last:
    case &ctr_array_min:
    case &ctr_array_max:
    case &ctr_array_sum:
    case &ctr_array_product:
    case &ctr_array_multiply:
    case &ctr_map_get:
    case &ctr_iterator_skip:
    case &ctr_iterator_next:
      return CtrStdObject;

    case &ctr_object_equals:
    case &ctr_bool_and:
    case &ctr_bool_nor:
    case &ctr_bool_or:
    case &ctr_bool_eq:
    case &ctr_bool_neq:
    case &ctr_bool_xor:
    case &ctr_bool_not:
    case &ctr_bool_flip:
    case &ctr_bool_break:
    case &ctr_bool_continue:
    case &ctr_bool_assign:
    case &ctr_number_higherThan:
    case &ctr_number_higherEqThan:
    case &ctr_number_lowerThan:
    case &ctr_number_lowerEqThan:
    case &ctr_number_eq:
    case &ctr_number_neq:
    case &ctr_number_odd:
    case &ctr_number_even:
    case &ctr_number_to_boolean:
    case &ctr_number_between:
    case &ctr_number_positive:
    case &ctr_number_negative:
    case &ctr_string_eq:
    case &ctr_string_neq:
    case &ctr_string_to_boolean:
    case &ctr_string_is_regex_pcre:
    case &ctr_string_contains_pattern:
    case &ctr_string_contains:
    case &ctr_string_is_ctor:
    case &ctr_object_to_boolean:
    case &ctr_object_is_nil:
    case &ctr_nil_to_boolean:
      return CtrStdBool;

    case &ctr_object_to_number:
    case &ctr_bool_to_number:
    case &ctr_nil_to_number:
    case &ctr_number_add:
    case &ctr_number_inc:
    case &ctr_number_minus:
    case &ctr_number_dec:
    case &ctr_number_multiply:
    case &ctr_number_times:
    case &ctr_number_mul:
    case &ctr_number_divide:
    case &ctr_number_div:
    case &ctr_number_modulo:
    case &ctr_number_factorial:
    case &ctr_number_floor:
    case &ctr_number_ceil:
    case &ctr_number_round:
    case &ctr_number_abs:
    case &ctr_number_sin:
    case &ctr_number_cos:
    case &ctr_number_exp:
    case &ctr_number_sqrt:
    case &ctr_number_tan:
    case &ctr_number_atan:
    case &ctr_number_log:
    case &ctr_number_shr:
    case &ctr_number_shl:
    case &ctr_number_pow:
    case &ctr_number_min:
    case &ctr_number_max:
    case &ctr_number_xor:
    case &ctr_number_or:
    case &ctr_number_and:
    case &ctr_number_to_step_do:
    case &ctr_number_to_byte:
    case &ctr_number_qualify:
    case &ctr_number_assign:
    case &ctr_string_length:
    case &ctr_string_byte_at:
    case &ctr_string_index_of:
    case &ctr_string_last_index_of:
    case &ctr_string_to_number:
    case &ctr_string_hash_with_key:
    case &ctr_iterator_count:
    case &ctr_map_count:
      return CtrStdNumber;

    case &ctr_string_bytes:
    case &ctr_string_split:
    case &ctr_string_characters:
    case &ctr_string_to_byte_array:
    case &ctr_array_new:
    case &ctr_array_copy:
    case &ctr_array_new_and_push:
    case &ctr_array_fmap:
    case &ctr_array_imap:
    case &ctr_array_filter:
    case &ctr_array_push:
    case &ctr_array_push_imm:
    case &ctr_array_reverse:
    case &ctr_array_sort:
    case &ctr_array_put:
    case &ctr_array_from_length:
    case &ctr_array_tail:
    case &ctr_array_init:
    case &ctr_array_add:
    case &ctr_array_map:
    case &ctr_array_fill:
    case &ctr_array_column:
    case &ctr_array_assign:
    case &ctr_iterator_fmap:
    case &ctr_array_intersperse:
    case &ctr_iterator_take:
    case &ctr_iterator_takewhile:
    case &ctr_iterator_to_array:
      return CtrStdArray;

    case &ctr_block_set:
    case &ctr_block_catch:
    case &ctr_block_while_true:
    case &ctr_block_while_false:
    case &ctr_block_times:
    case &ctr_block_to_string:
    case &ctr_block_assign:
      return CtrStdBlock;

    case &ctr_map_new:
    case &ctr_map_put:
    case &ctr_map_each:
    case &ctr_map_kvmap:
    case &ctr_map_kvlist:
    case &ctr_map_flip:
    case &ctr_map_assign:
      return CtrStdMap;

    case &ctr_iterator_make:
    case &ctr_iterator_set_seed:
    case &ctr_iterator_set_func:
    case &ctr_iterator_make_range:
    case &ctr_iterator_make_uncapped_range:
    case &ctr_iterator_each:
    case &ctr_iterator_end:
    case &ctr_iterator_end_check:
      return CtrStdIter;

    case &ctr_console_write:
    case &ctr_console_red:
    case &ctr_console_green:
    case &ctr_console_yellow:
    case &ctr_console_blue:
    case &ctr_console_magenta:
    case &ctr_console_cyan:
    case &ctr_console_reset:
    case &ctr_console_tab:
    case &ctr_console_line:
    case &ctr_console_clear:
    case &ctr_console_clear_line:
      return CtrStdConsole;

    case &ctr_file_new:
    case &ctr_file_path:
    case &ctr_file_read:
    case &ctr_file_write:
    case &ctr_file_append:
    case &ctr_file_exists:
    case &ctr_file_size:
    case &ctr_file_delete:
    case &ctr_file_include:
    case &ctr_file_include_here:
    case &ctr_file_open:
    case &ctr_file_close:
    case &ctr_file_read_bytes:
    case &ctr_file_write_bytes:
    case &ctr_file_seek:
    case &ctr_file_seek_rewind:
    case &ctr_file_seek_end:
    case &ctr_file_descriptor:
    case &ctr_file_lock_generic:
    case &ctr_file_lock:
    case &ctr_file_unlock:
    case &ctr_file_list:
    case &ctr_file_tmp:
    case &ctr_file_stdext_path:
    case &ctr_command_argument:
    case &ctr_command_num_of_args:
    case &ctr_command_waitforinput:
    case &ctr_command_getc:
    case &ctr_command_input:
    case &ctr_command_set_INT_handler:
    case &ctr_command_get_env:
    case &ctr_command_set_env:
    case &ctr_command_exit:
    case &ctr_command_flush:
    case &ctr_command_forbid_shell:
    case &ctr_command_forbid_file_write:
    case &ctr_command_forbid_file_read:
    case &ctr_command_forbid_include:
    case &ctr_command_forbid_fork:
    case &ctr_command_countdown:
    case &ctr_command_fork:
    case &ctr_command_message:
    case &ctr_command_listen:
    case &ctr_command_join:
    case &ctr_command_log:
    case &ctr_command_warn:
    case &ctr_command_err:
    case &ctr_command_crit:
    case &ctr_command_pid:
    case &ctr_command_accept:
    case &ctr_command_accept_number:
    case &ctr_command_remote:
    case &ctr_command_default_port:
    case &ctr_clock_change:
    case &ctr_clock_wait:
    case &ctr_clock_time:
    case &ctr_clock_new:
    case &ctr_clock_new_set:
    case &ctr_clock_like:
    case &ctr_clock_day:
    case &ctr_clock_month:
    case &ctr_clock_year:
    case &ctr_clock_hour:
    case &ctr_clock_minute:
    case &ctr_clock_second:
    case &ctr_clock_weekday:
    case &ctr_clock_yearday:
    case &ctr_clock_week:
    case &ctr_clock_set_day:
    case &ctr_clock_set_month:
    case &ctr_clock_set_year:
    case &ctr_clock_set_hour:
    case &ctr_clock_set_minute:
    case &ctr_clock_set_second:
    case &ctr_clock_get_time:
    case &ctr_clock_set_time:
    case &ctr_clock_set_zone:
    case &ctr_clock_get_zone:
    case &ctr_clock_to_string:
    case &ctr_clock_format:
    case &ctr_clock_add:
    case &ctr_clock_subtract:
    case &ctr_clock_processor_time:
    case &ctr_clock_processor_ticks_ps:
    case &ctr_clock_time_exec:
    case &ctr_clock_time_exec_s:
    case &ctr_slurp_obtain:
    case &ctr_slurp_respond_to:
    case &ctr_slurp_respond_to_and:
    case &ctr_shell_call:
    case &ctr_shell_respond_to:
    case &ctr_shell_respond_to_and:
    case &ctr_gc_collect:
    case &ctr_gc_dust:
    case &ctr_gc_object_count:
    case &ctr_gc_kept_count:
    case &ctr_gc_kept_alloc:
    case &ctr_gc_sticky_count:
    case &ctr_gc_setmode:
    case &ctr_gc_setmemlimit:
    case &ctr_reflect_add_glob:
    case &ctr_reflect_add_local:
    case &ctr_reflect_add_my:
    case &ctr_reflect_dump_context_spec_prop:
    case &ctr_reflect_dump_context_spec:
    case &ctr_reflect_dump_context:
    case &ctr_reflect_find_obj:
    case &ctr_reflect_find_obj_ex:
    case &ctr_reflect_new:
    case &ctr_reflect_set_to:
    case &ctr_reflect_cb_ac:
    case &ctr_reflect_cb_add_param:
    case &ctr_reflect_fn_copy:
    case &ctr_reflect_share_memory:
    case &ctr_reflect_link_to:
    case &ctr_reflect_child_of:
    case &ctr_reflect_is_linked_to:
    case &ctr_reflect_generate_inheritance_tree:
    case &ctr_reflect_describe_type:
    case &ctr_reflect_describe_value:
    case &ctr_reflect_type_descriptor_print:
    case &ctr_reflect_bind:
    case &ctr_reflect_cons_of:
    case &ctr_reflect_try_serialize_block:
    case &ctr_reflect_cons_value:
    case &ctr_reflect_cons_str:
    case &ctr_fiber_spawn:
    case &ctr_fiber_yield:
    case &ctr_fiber_join_all:
    case &ctr_fiber_tostring:
    case &ctr_fiber_yielded:
    case &ctr_dice_throw:
    case &ctr_dice_sides:
    case &ctr_dice_rand:
    default:
      return CtrStdObject;
  }
}