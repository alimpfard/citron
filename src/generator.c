#include "generator.h"
#include "citron.h"
#include <stdlib.h>

const static ctr_object generator_end_marker_o;
ctr_object* generator_end_marker = (ctr_object*)&generator_end_marker_o;

void ctr_condense_generator(ctr_generator*, int);
ctr_mapping_generator* ctr_combine_generators(ctr_mapping_generator*,
    ctr_mapping_generator*);
ctr_object* ctr_generator_internal_inext(ctr_generator* genny, int gtype,
    ctr_generator* ogenny, int ogtype);

/**@I_OBJ_DEF Generator*/
/**
 * [Generator] from: [Number] to: [Number] [step: [Number]]
 *
 * Creates a numeric step generator
 */
/**
 * [Generator] elementsOf: [Array|Map|String]
 *
 * Creates a generator that steps through the elements of a collection
 */
ctr_object* ctr_generator_make(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_object* inst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_set_link_all(inst, myself);
    inst->release_hook = ctr_generator_free;
    inst->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
    ctr_resource* res = inst->value.rvalue;
    ctr_object* arg = argumentList->object;
    switch (arg->info.type) {
    case CTR_OBJECT_TYPE_OTNUMBER: {
        ctr_object *end, *step;
        if (argumentList->next && (end = argumentList->next->object)) { // from_to_
            res->type = CTR_STEP_GENNY;
            if (argumentList->next->next && (step = argumentList->next->next->object)) { // from_to_step
                ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
                gen->seq_index = 0;
                ctr_step_generator* sg = ctr_heap_allocate(sizeof(ctr_step_generator));
                sg->current = arg->value.nvalue;
                sg->end = end->value.nvalue;
                sg->step = step->value.nvalue;
                gen->data = NULL;
                gen->finished = 0;
                gen->sequence = sg;
                res->ptr = gen;
            } else { // from_to_
                ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
                gen->seq_index = 0;
                ctr_step_generator* sg = ctr_heap_allocate(sizeof(ctr_step_generator));
                sg->current = arg->value.nvalue;
                sg->end = end->value.nvalue;
                sg->step = (ctr_number)1;
                gen->data = NULL;
                gen->finished = 0;
                gen->sequence = sg;
                res->ptr = gen;
            }
        } else { // repeat
            res->type = CTR_REPEAT_GENNY;
            ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
            gen->finished = 0;
            gen->sequence = arg;
            res->ptr = gen;
        }
        break;
    }
    case CTR_OBJECT_TYPE_OTSTRING: { // elements_of
        res->type = CTR_E_OF_S_GENNY;
        ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
        gen->finished = 0;
        gen->seq_index = 0;
        gen->sequence = arg->value.svalue;
        res->ptr = gen;
        break;
    }
    case CTR_OBJECT_TYPE_OTARRAY: { // elements_of
        res->type = CTR_E_OF_A_GENNY;
        ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
        gen->finished = 0;
        gen->seq_index = 0;
        gen->sequence = arg->value.avalue;
        res->ptr = gen;
        break;
    }
    case CTR_OBJECT_TYPE_OTEX:
        if (arg->interfaces->link == ctr_std_generator)
            return ctr_generator_copy(arg, NULL);
    default: {                                                  // repeat | map
        if (ctr_reflect_get_primitive_link(arg) == CtrStdMap) { // map
            res->type = CTR_E_OF_M_GENNY;
            ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
            gen->finished = 0;
            gen->seq_index = 0;
            gen->sequence = arg->properties;
            res->ptr = gen;
        } else { // simply repeat
            res->type = CTR_REPEAT_GENNY;
            ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
            gen->finished = 0;
            gen->sequence = arg;
            res->ptr = gen;
        }
    }
    }
    return inst;
}

/**
 * [Generator] repeat: [Object]
 *
 * Creates a generator that simply repeats an object forever
 */
ctr_object* ctr_generator_make_rept(ctr_object* myself,
    ctr_argument* argumentList)
{
    ctr_object* inst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_set_link_all(inst, myself);
    inst->release_hook = ctr_generator_free;
    inst->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
    ctr_resource* res = inst->value.rvalue;
    ctr_object* arg = argumentList->object;
    res->type = CTR_REPEAT_GENNY;
    ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
    gen->finished = 0;
    gen->sequence = arg;
    res->ptr = gen;
    return inst;
}

/**
 * [Generator] filter: [Block<idx,value>]
 *
 * Creates a generator that filters another generator through the given block
 */
ctr_object* ctr_generator_filter(ctr_object* myself,
    ctr_argument* argumentList)
{
    ctr_object* blk = argumentList->object;
    ctr_object* inst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_set_link_all(inst, ctr_std_generator);
    ctr_generator* under = myself->value.rvalue->ptr;
    inst->release_hook = ctr_generator_free;
    inst->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
    ctr_resource* res = inst->value.rvalue;
    res->type = CTR_FIL_OF_GENNY;
    ctr_mapping_generator* genny = ctr_heap_allocate(sizeof(*genny));
    genny->i_type = myself->value.rvalue->type;
    genny->genny = myself->value.rvalue->ptr;
    genny->fn = blk;
    ctr_generator* generator = ctr_heap_allocate(sizeof(*generator));
    generator->seq_index = under->seq_index;
    generator->sequence = genny;
    ctr_argument* argm = ctr_heap_allocate(sizeof(ctr_argument));
    argm->next = ctr_heap_allocate(sizeof(ctr_argument));
    generator->data = argm;
    generator->finished = under->finished;
    // ctr_condense_generator(generator, CTR_FN_OF_GENNY);
    res->ptr = generator;
    return inst;
}

/**
 * [Generator] imap: [Block]
 *
 * Creates a generator that maps the elements of this generator through the
 * given block.
 */
ctr_object* ctr_generator_imap(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_object* blk = argumentList->object;
    ctr_object* inst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_set_link_all(inst, ctr_std_generator);
    ctr_generator* under = myself->value.rvalue->ptr;
    inst->release_hook = ctr_generator_free;
    inst->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
    ctr_resource* res = inst->value.rvalue;
    res->type = CTR_FN_OF_GENNY;
    ctr_mapping_generator* genny = ctr_heap_allocate(sizeof(*genny));
    genny->i_type = myself->value.rvalue->type;
    genny->genny = myself->value.rvalue->ptr;
    genny->fn = blk;
    ctr_generator* generator = ctr_heap_allocate(sizeof(*generator));
    generator->seq_index = under->seq_index;
    generator->sequence = genny;
    ctr_argument* argm = ctr_heap_allocate(sizeof(ctr_argument));
    argm->next = ctr_heap_allocate(sizeof(ctr_argument));
    generator->data = argm;
    generator->finished = under->finished;
    // ctr_condense_generator(generator, CTR_FN_OF_GENNY);
    res->ptr = generator;
    return inst;
}

/**
 * [Generator] fmap: [Block]
 *
 * Creates a generator that maps the elements of this generator through the
 * given block.
 */
ctr_object* ctr_generator_fmap(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_object* blk = argumentList->object;
    if (!blk || blk->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
        CtrStdFlow = ctr_build_string_from_cstring("Expected a block");
        return myself;
    }
    ctr_object* inst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_set_link_all(inst, ctr_std_generator);
    ctr_generator* under = myself->value.rvalue->ptr;
    inst->release_hook = ctr_generator_free;
    inst->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
    ctr_resource* res = inst->value.rvalue;
    res->type = CTR_IFN_OF_GENNY;
    ctr_mapping_generator* genny = ctr_heap_allocate(sizeof(*genny));
    genny->i_type = myself->value.rvalue->type;
    genny->genny = myself->value.rvalue->ptr;
    genny->fn = blk;
    ctr_generator* generator = ctr_heap_allocate(sizeof(*generator));
    generator->seq_index = under->seq_index;
    generator->sequence = genny;
    ctr_argument* argm = ctr_heap_allocate(sizeof(ctr_argument));
    argm->next = ctr_heap_allocate(sizeof(ctr_argument));
    generator->data = argm;
    generator->finished = under->finished;
    // ctr_condense_generator(generator, CTR_FN_OF_GENNY);
    res->ptr = generator;
    return inst;
}

/**
 * [Generator] ifmap: [Block]
 *
 * Creates a generator that maps the elements of this generator through the
 * given block; should the block yield a generator, it will be depleted, and its
 * elements unpacked recursively
 */
ctr_object* ctr_generator_ifmap(ctr_object* myself,
    ctr_argument* argumentList)
{
    ctr_object* blk = argumentList->object;
    if (!blk || blk->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
        CtrStdFlow = ctr_build_string_from_cstring("Expected a block");
        return myself;
    }
    ctr_object* inst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_set_link_all(inst, ctr_std_generator);
    ctr_generator* under = myself->value.rvalue->ptr;
    inst->release_hook = ctr_generator_free;
    inst->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
    ctr_resource* res = inst->value.rvalue;
    res->type = CTR_XFN_OF_GENNY;
    ctr_mapping_generator* genny = ctr_heap_allocate(sizeof(*genny));
    genny->i_type = myself->value.rvalue->type;
    genny->genny = myself->value.rvalue->ptr;
    genny->fn = blk;
    ctr_generator* generator = ctr_heap_allocate(sizeof(*generator));
    generator->seq_index = under->seq_index;
    generator->sequence = genny;
    ctr_argument* argm = ctr_heap_allocate(sizeof(ctr_argument));
    argm->next = ctr_heap_allocate(sizeof(ctr_argument));
    generator->data = argm;
    generator->finished = under->finished;
    res->ptr = generator;
    return inst;
}

ctr_object* ctr_generator_internal_next(ctr_generator* genny, int gtype)
{
    if (genny->finished)
        return generator_end_marker;

    switch (gtype) {
    case CTR_STEP_GENNY: {
        genny->seq_index++;
        ctr_step_generator* sg = genny->sequence;
        ctr_object* num = ctr_build_number_from_float(sg->current);
        if (sg->end < sg->current) {
            genny->finished = 1;
        }
        sg->current += sg->step;
        return num;
    }
    case CTR_REPEAT_GENNY: {
        return genny->sequence;
    }
    case CTR_E_OF_S_GENNY: {
        ctr_string* str = genny->sequence;
        if (genny->seq_index >= str->vlen) {
            genny->finished = 1;
            return generator_end_marker;
        }
        long ua = getBytesUtf8(str->value, 0, genny->seq_index);
        long ub = getBytesUtf8(str->value, ua, 1);
        ctr_object* newString;
        char* dest = ctr_heap_allocate(ub * sizeof(char));
        memcpy(dest, (str->value) + ua, ub);
        newString = ctr_build_string(dest, ub);
        ctr_heap_free(dest);
        genny->seq_index++;
        return newString;
    }
    case CTR_E_OF_A_GENNY: {
        ctr_collection* coll = genny->sequence;
        if (genny->seq_index >= coll->head - coll->tail) {
            genny->finished = 1;
            return generator_end_marker;
        }
        ctr_object* elem = coll->elements[genny->seq_index];
        genny->seq_index++;
        return elem;
    }
    case CTR_E_OF_M_GENNY: {
        ctr_map* map = genny->sequence;
        if (genny->seq_index >= map->size) {
            genny->finished = 1;
            return generator_end_marker;
        }
        if (!genny->data) {
            genny->data = ctr_heap_allocate(sizeof(ctr_argument));
        }
        ctr_size c = genny->seq_index++;
        ctr_mapitem* head = map->head;
        while (--c > 0) {
            head = head->next;
        }
        ctr_object* tup = ctr_array_new(CtrStdArray, NULL);
        ctr_argument* argm = genny->data;
        argm->object = head->key;
        ctr_array_push(tup, argm);
        argm->object = head->value;
        ctr_array_push(tup, argm);
        tup->value.avalue->immutable = 1;
        return tup;
    }
    case CTR_FN_OF_GENNY: {
        ctr_mapping_generator* mgen = genny->sequence;
        ctr_argument* argm = genny->data;
        ctr_generator* igen = mgen->genny;
        int igen_type = mgen->i_type;
        ctr_object* fn = mgen->fn;
        argm->object = ctr_build_number_from_float(genny->seq_index++);
        do {
            argm->next->object = ctr_generator_internal_next(igen, igen_type);
        } while (argm->next->object == generator_end_marker && !igen->finished);
        genny->finished = genny->finished || igen->finished;
        if (argm->next->object == generator_end_marker)
            return argm->next->object;
        ctr_object* res = ctr_block_run(fn, argm, fn);
        if (CtrStdFlow) {
            if (CtrStdFlow == CtrStdContinue) {
                CtrStdFlow = NULL;
                return ctr_generator_internal_next(genny, gtype);
            }
            if (CtrStdFlow == CtrStdBreak) {
                CtrStdFlow = NULL;
                genny->finished = 1;
            }
        }
        return genny->current = res;
    }
    case CTR_XFN_OF_GENNY: {
        ctr_mapping_generator* mgen = genny->sequence;
        ctr_argument* argm = genny->data;
        ctr_generator* igen = mgen->genny;
        int igen_type = mgen->i_type;
        ctr_object* fn = mgen->fn;
        genny->seq_index++;
        do {
            argm->object = ctr_generator_internal_inext(igen, igen_type, NULL, 0);
        } while (argm->object == generator_end_marker && !igen->finished);
        genny->finished = genny->finished || igen->finished;
        if (argm->object == generator_end_marker)
            return argm->object;
        ctr_object* res = ctr_block_run(fn, argm, fn);
        if (CtrStdFlow) {
            if (CtrStdFlow == CtrStdContinue) {
                CtrStdFlow = NULL;
                return ctr_generator_internal_inext(genny, gtype, NULL, 0);
            }
            if (CtrStdFlow == CtrStdBreak) {
                CtrStdFlow = NULL;
                genny->finished = 1;
            }
        }
        return genny->current = res;
    }
    case CTR_IFN_OF_GENNY: {
        ctr_mapping_generator* mgen = genny->sequence;
        ctr_argument* argm = genny->data;
        ctr_generator* igen = mgen->genny;
        int igen_type = mgen->i_type;
        ctr_object* fn = mgen->fn;
        genny->seq_index++;
        do {
            argm->object = ctr_generator_internal_next(igen, igen_type);
        } while (argm->object == generator_end_marker && !igen->finished);
        genny->finished = genny->finished || igen->finished;
        if (argm->object == generator_end_marker)
            return argm->object;
        ctr_object* res = ctr_block_run(fn, argm, fn);
        if (CtrStdFlow) {
            if (CtrStdFlow == CtrStdContinue) {
                CtrStdFlow = NULL;
                return ctr_generator_internal_next(genny, gtype);
            }
            if (CtrStdFlow == CtrStdBreak) {
                CtrStdFlow = NULL;
                genny->finished = 1;
            }
        }
        return genny->current = res;
    }
    case CTR_FIL_OF_GENNY: {
        ctr_mapping_generator* mgen = genny->sequence;
        ctr_argument* argm = genny->data;
        ctr_generator* igen = mgen->genny;
        int igen_type = mgen->i_type;
        ctr_object* fn = mgen->fn;
        argm->object = ctr_build_number_from_float(genny->seq_index++);
        do {
            argm->next->object = ctr_generator_internal_next(igen, igen_type);
        } while ((argm->next->object == generator_end_marker && !igen->finished) || (argm->next->object != generator_end_marker && !ctr_block_run(fn, argm, fn)->value.bvalue));
        genny->finished = genny->finished || igen->finished;
        if (argm->next->object == generator_end_marker)
            return argm->next->object;
        if (CtrStdFlow) {
            if (CtrStdFlow == CtrStdContinue) {
                CtrStdFlow = NULL;
                return ctr_generator_internal_next(genny, gtype);
            }
            if (CtrStdFlow == CtrStdBreak) {
                CtrStdFlow = NULL;
                genny->finished = 1;
            }
        }
        return genny->current = argm->next->object;
    }
    default:
        return NULL;
    }
}

ctr_object* ctr_generator_internal_inext(ctr_generator* genny, int gtype,
    ctr_generator* ogenny, int ogtype)
{
    ctr_object* current = genny->current;
    if (current == generator_end_marker)
        current = ctr_generator_internal_next(genny, gtype);
    int isnext = 0;
    int fail = 0;
    if ((isnext = !current))
        current = ctr_generator_internal_next(genny, gtype);

    while (current == generator_end_marker && !genny->finished) {
        current = ctr_generator_internal_next(genny, gtype);
    }
    if (current == generator_end_marker) {
        return current;
    }
    if (current->interfaces->link == ctr_std_generator) {
        ctr_generator* gen = current->value.rvalue->ptr;
        int gty = current->value.rvalue->type;
        if (!gen->finished) {
            ctr_object* res = ctr_generator_internal_inext(gen, gty, genny, gtype);
            if (res == generator_end_marker) {
                ctr_generator_internal_next(genny, gtype);
                return ctr_generator_internal_inext(genny, gtype, NULL, 0);
            } else
                return res;
        }
        if (ogenny) {
            ctr_generator_internal_next(ogenny, ogtype);
            return ctr_generator_internal_inext(ogenny, ogtype, NULL, 0);
        }
        fail = 1;
    }
    current = isnext ? current : ctr_generator_internal_next(genny, gtype);
    return fail ? generator_end_marker : current;
}

/**
 * [Generator] next
 *
 * Gets the next element of this generator
 */
ctr_object* ctr_generator_next(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_resource* res = myself->value.rvalue;
    ctr_generator* genny = res->ptr;
    int gtype = res->type;
    if (!genny) {
        CtrStdFlow = ctr_build_string_from_cstring("#next on uninitialized generator");
        return CtrStdNil;
    }
    ctr_object* next = ctr_generator_internal_next(genny, gtype);
    if (!next) {
        CtrStdFlow = ctr_build_string_from_cstring("Invalid generator type(probably)");
        return CtrStdNil;
    }
    if (next == generator_end_marker) {
        genny->finished = 1;
        return CtrStdNil;
    }
    return next;
}

/**
 * [Generator] inext
 *
 * Gets the next element of this generator; should it be a generator, it will
 * first be recursively used, and itself ignored
 */
ctr_object* ctr_generator_inext(ctr_object* myself,
    ctr_argument* argumentList)
{
    ctr_resource* res = myself->value.rvalue;
    ctr_generator* genny = res->ptr;
    int gtype = res->type;
    if (!genny) {
        CtrStdFlow = ctr_build_string_from_cstring("#inext on uninitialized generator");
        return CtrStdNil;
    }
    ctr_object* next = ctr_generator_internal_inext(genny, gtype, NULL, 0);
    if (!next) {
        CtrStdFlow = ctr_build_string_from_cstring("Invalid generator type(probably)");
        return CtrStdNil;
    }
    if (next == generator_end_marker) {
        genny->finished = 1;
        return CtrStdNil;
    }
    return next;
}

/**
 * [Generator] finished
 *
 * Returns whether this generator is done producing items
 */
ctr_object* ctr_generator_isfin(ctr_object* myself,
    ctr_argument* argumentList)
{
    ctr_resource* res = myself->value.rvalue;
    ctr_generator* genny = res->ptr;
    if (!genny)
        return ctr_build_bool(1);
    return ctr_build_bool(genny->finished);
}

/**
 * [Generator] finish
 *
 * Finish the generator
 */
ctr_object* ctr_generator_fin(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_resource* res = myself->value.rvalue;
    ctr_generator* genny = res->ptr;
    if (genny)
        genny->finished = 1;
    return myself;
}

/**
 * [Generator] each: [Block]
 *
 * Runs the block for each element in the generator
 */
ctr_object* ctr_generator_each(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_object* blk = argumentList->object;
    ctr_resource* res = myself->value.rvalue;
    ctr_generator* genny = res->ptr;
    int gtype = res->type;
    if (!genny) {
        CtrStdFlow = ctr_build_string_from_cstring("#next on uninitialized generator");
        return CtrStdNil;
    }
    ctr_argument* argm = ctr_heap_allocate(sizeof(*argm));
    argm->next = ctr_heap_allocate(sizeof(*argm));
    while (1) {
        ctr_object* next = ctr_generator_internal_next(genny, gtype);
        if (genny->finished)
            break;
        if (next == generator_end_marker)
            continue;
        argm->object = ctr_build_number_from_float(genny->seq_index - 1);
        argm->next->object = next;
        ctr_block_run(blk, argm, blk);
        if (CtrStdFlow) {
            if (CtrStdFlow == CtrStdContinue) {
                CtrStdFlow = NULL;
                continue;
            }
            if (CtrStdFlow == CtrStdBreak) {
                CtrStdFlow = NULL;
                genny->finished = 1;
            }
            break;
        }
    }
    if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow = NULL;

    ctr_heap_free(argm->next);
    ctr_heap_free(argm);
    return myself;
}

/**
 * [Generator] each_v: [Block]
 *
 * Runs the block for each element in the generator
 */
ctr_object* ctr_generator_eachv(ctr_object* myself,
    ctr_argument* argumentList)
{
    ctr_object* blk = argumentList->object;
    ctr_resource* res = myself->value.rvalue;
    ctr_generator* genny = res->ptr;
    int gtype = res->type;
    if (!genny) {
        CtrStdFlow = ctr_build_string_from_cstring("#next on uninitialized generator");
        return CtrStdNil;
    }
    ctr_argument* argm = ctr_heap_allocate(sizeof(*argm));
    while (1) {
        ctr_object* next = ctr_generator_internal_next(genny, gtype);
        if (genny->finished)
            break;
        if (next == generator_end_marker)
            continue;
        argm->object = next;
        ctr_block_run(blk, argm, blk);
        if (CtrStdFlow) {
            if (CtrStdFlow == CtrStdContinue) {
                CtrStdFlow = NULL;
                continue;
            }
            if (CtrStdFlow == CtrStdBreak) {
                CtrStdFlow = NULL;
                genny->finished = 1;
            }
            break;
        }
    }
    if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow = NULL;
    ctr_heap_free(argm);
    return myself;
}

/**
 * [Generator] ieach: [Block]
 *
 * Runs the block for each element in the generator;
 * should the generator yield another generator, it will be depleted, and its
 * elements unpacked recursively
 */
ctr_object* ctr_generator_ieach(ctr_object* myself,
    ctr_argument* argumentList)
{
    ctr_object* blk = argumentList->object;
    ctr_resource* res = myself->value.rvalue;
    ctr_generator* genny = res->ptr;
    int gtype = res->type;
    if (!genny) {
        CtrStdFlow = ctr_build_string_from_cstring("#inext on uninitialized generator");
        return CtrStdNil;
    }
    ctr_argument* argm = ctr_heap_allocate(sizeof(*argm));
    argm->next = ctr_heap_allocate(sizeof(*argm));
    while (1) {
        ctr_object* next = ctr_generator_internal_inext(genny, gtype, NULL, 0);
        if (genny->finished)
            break;
        if (next == generator_end_marker)
            continue;
        argm->object = ctr_build_number_from_float(genny->seq_index - 1);
        argm->next->object = next;
        ctr_block_run(blk, argm, blk);
        if (CtrStdFlow) {
            if (CtrStdFlow == CtrStdContinue) {
                CtrStdFlow = NULL;
                continue;
            }
            if (CtrStdFlow == CtrStdBreak) {
                CtrStdFlow = NULL;
                genny->finished = 1;
            }
            break;
        }
    }
    if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow = NULL;
    ctr_heap_free(argm->next);
    ctr_heap_free(argm);
    return myself;
}

/**
 * [Generator] ieach_v: [Block]
 *
 * Runs the block for each element in the generator;
 * should the generator yield another generator, it will be depleted, and its
 * elements unpacked recursively
 */
ctr_object* ctr_generator_ieachv(ctr_object* myself,
    ctr_argument* argumentList)
{
    ctr_object* blk = argumentList->object;
    ctr_resource* res = myself->value.rvalue;
    ctr_generator* genny = res->ptr;
    int gtype = res->type;
    if (!genny) {
        CtrStdFlow = ctr_build_string_from_cstring("#inext on uninitialized generator");
        return CtrStdNil;
    }
    ctr_argument* argm = ctr_heap_allocate(sizeof(*argm));
    argm->next = NULL;
    while (1) {
        ctr_object* next = ctr_generator_internal_inext(genny, gtype, NULL, 0);
        if (genny->finished)
            break;
        if (next == generator_end_marker)
            continue;
        argm->object = next;
        ctr_block_run(blk, argm, blk);
        if (CtrStdFlow) {
            if (CtrStdFlow == CtrStdContinue) {
                CtrStdFlow = NULL;
                continue;
            }
            if (CtrStdFlow == CtrStdBreak) {
                CtrStdFlow = NULL;
                genny->finished = 1;
            }
            break;
        }
    }
    if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow = NULL;
    ctr_heap_free(argm);
    return myself;
}

ctr_object* ctr_generator_internal_tostr(ctr_generator* gen, int gtype)
{
    switch (gtype) {
    case CTR_REPEAT_GENNY:
        return ctr_build_string_from_cstring("[RepeatGenerator]");
    case CTR_E_OF_S_GENNY:
        return ctr_build_string_from_cstring("[StringGenerator]");
    case CTR_E_OF_A_GENNY:
        return ctr_build_string_from_cstring("[ArrayGenerator]");
    case CTR_STEP_GENNY:
        return ctr_build_string_from_cstring("[StepGenerator]");
    case CTR_E_OF_M_GENNY:
        return ctr_build_string_from_cstring("[MapGenerator]");
    case CTR_FN_OF_GENNY:
    case CTR_IFN_OF_GENNY:
    case CTR_XFN_OF_GENNY: {
        ctr_object* str = ctr_build_string_from_cstring("[MappedGenerator<");
        ctr_mapping_generator* mgen = gen->sequence;
        ctr_object* inner = ctr_generator_internal_tostr(mgen->genny, mgen->i_type);
        ctr_argument* arg = gen->data;
        arg->object = inner;
        ctr_string_append(str, arg);
        arg->object = ctr_build_string_from_cstring(">]");
        ctr_string_append(str, arg);
        return str;
    }
    case CTR_FIL_OF_GENNY: {
        ctr_object* str = ctr_build_string_from_cstring("[FilteredGenerator<");
        ctr_mapping_generator* mgen = gen->sequence;
        ctr_object* inner = ctr_generator_internal_tostr(mgen->genny, mgen->i_type);
        ctr_argument* arg = gen->data;
        arg->object = inner;
        ctr_string_append(str, arg);
        arg->object = ctr_build_string_from_cstring(">]");
        ctr_string_append(str, arg);
        return str;
    }
    default:
        return ctr_build_string_from_cstring("[Generator]");
    }
}

/**
 * [Generator] toString
 *
 * Returns a string representation of this generator
 */
ctr_object* ctr_generator_tostr(ctr_object* myself,
    ctr_argument* argumentList)
{
    if (!myself->value.rvalue)
        return ctr_build_string_from_cstring("[UninitializedGenerator]");
    return ctr_generator_internal_tostr(myself->value.rvalue->ptr,
        myself->value.rvalue->type);
}

ctr_generator* ctr_internal_generator_copy(ctr_generator* genny, int gtype)
{
    ctr_generator* gcopy = ctr_heap_allocate(sizeof(ctr_generator));
    switch (gtype) {
    case CTR_STEP_GENNY: {
        *gcopy = *genny;
        ctr_step_generator* mgen = gcopy->sequence;
        gcopy->sequence = ctr_heap_allocate(sizeof(*mgen));
        *((ctr_step_generator*)(gcopy->sequence)) = *mgen;
        break;
    }
    case CTR_REPEAT_GENNY:
    case CTR_E_OF_S_GENNY:
    case CTR_E_OF_A_GENNY:
    case CTR_E_OF_M_GENNY:
        *gcopy = *genny;
        break;
    case CTR_FN_OF_GENNY:
    case CTR_IFN_OF_GENNY:
    case CTR_XFN_OF_GENNY: {
        *gcopy = *genny;
        ctr_mapping_generator* mgen = gcopy->sequence;
        gcopy->sequence = ctr_heap_allocate(sizeof(*mgen));
        ((ctr_mapping_generator*)(gcopy->sequence))->genny = ctr_internal_generator_copy(mgen->genny, mgen->i_type);
        ((ctr_mapping_generator*)(gcopy->sequence))->fn = mgen->fn;
        break;
    }
    }
    return gcopy;
}

/**
 * [Generator] toArray
 *
 * Unpacks the contents of this generator to an array
 */
ctr_object* ctr_generator_toarray(ctr_object* myself,
    ctr_argument* argumentList)
{
    ctr_resource* res = myself->value.rvalue;
    ctr_generator* genny = res->ptr;
    int gtype = res->type;
    ctr_object* array = ctr_array_new(CtrStdArray, NULL);
    if (!genny) {
        CtrStdFlow = ctr_build_string_from_cstring("#inext on uninitialized generator");
        return CtrStdNil;
    }
    ctr_argument* argm = ctr_heap_allocate(sizeof(*argm));
    while (1) {
        ctr_object* next = ctr_generator_internal_inext(genny, gtype, NULL, 0);
        if (genny->finished)
            break;
        if (next == generator_end_marker)
            continue;
        argm->object = next;
        array = ctr_array_push(array, argm);
        if (CtrStdFlow) {
            if (CtrStdFlow == CtrStdContinue) {
                CtrStdFlow = NULL;
                continue;
            }
            if (CtrStdFlow == CtrStdBreak) {
                CtrStdFlow = NULL;
                genny->finished = 1;
            }
            break;
        }
    }
    if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow = NULL;
    ctr_heap_free(argm);
    return array;
}

/**
 * [Generator] copy
 *
 * Copies this generator to create another unique one
 * The state of this generator _will_ be preserved
 */
ctr_object* ctr_generator_copy(ctr_object* myself, ctr_argument* argumentList)
{
    if (!myself->value.rvalue)
        return myself;
    ctr_resource* res = myself->value.rvalue;
    ctr_generator *genny = res->ptr, *gcopy;
    int gtype = res->type;
    gcopy = ctr_internal_generator_copy(genny, gtype);
    ctr_object* inst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_set_link_all(inst, ctr_std_generator);
    inst->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
    inst->value.rvalue->ptr = gcopy;
    inst->value.rvalue->type = gtype;
    return inst;
}

ctr_generator* ctr_unwrap_generator(ctr_mapping_generator* mg)
{
    if (mg->i_type == CTR_FN_OF_GENNY)
        return ctr_unwrap_generator(mg->genny->sequence);
    return mg->genny;
}

void ctr_condense_generator(ctr_generator* gen, int ty)
{
    switch (ty) {
    case CTR_REPEAT_GENNY:
    case CTR_E_OF_S_GENNY:
    case CTR_E_OF_A_GENNY:
    case CTR_STEP_GENNY:
    case CTR_E_OF_M_GENNY:
        return;
    case CTR_FN_OF_GENNY: {
        ctr_mapping_generator* mgen = gen->sequence;
        if (mgen->i_type == CTR_FN_OF_GENNY) {
            ctr_condense_generator(mgen->genny, CTR_FN_OF_GENNY);
            gen->sequence = ctr_combine_generators(mgen, mgen->genny->sequence);
            mgen->genny = ctr_unwrap_generator(mgen);
        }
    }
    }
}

static ctr_object* fv_gen_combinator_str = NULL;
static ctr_object* fv_gen_str_0 = NULL;
static ctr_object* fv_gen_str_1 = NULL;
ctr_mapping_generator* ctr_combine_generators(ctr_mapping_generator* genp,
    ctr_mapping_generator* genc)
{
    // if(genp->fn->info.type == CTR_OBJECT_TYPE_OTBLOCK && genc->fn->info.type ==
    // CTR_OBJECT_TYPE_OTBLOCK) {
    if (fv_gen_combinator_str == NULL) {
        fv_gen_combinator_str = ctr_build_string_from_cstring(
            "{:fv ^my fn0 applyTo: (my fn1 applyTo: fv).}");
        fv_gen_str_0 = ctr_build_string_from_cstring("fn0");
        fv_gen_str_1 = ctr_build_string_from_cstring("fn1");
    }
    ctr_object* callmsg = ctr_string_eval(fv_gen_combinator_str, NULL);
    ctr_internal_object_set_property(callmsg, fv_gen_str_0, genp->fn, 0);
    ctr_internal_object_set_property(callmsg, fv_gen_str_1, genc->fn, 0);
    // }
    genp->fn = callmsg;
    return genp;
}

void* ctr_generator_free(void* res_)
{
    ctr_resource* res = res_;
    switch (res->type) {
    case CTR_REPEAT_GENNY: /* fall through */
    case CTR_E_OF_S_GENNY: /* fall through */
    case CTR_E_OF_A_GENNY:
        ctr_heap_free(res->ptr);
        return NULL;
    case CTR_STEP_GENNY: /* fall through */
    case CTR_E_OF_M_GENNY:
        ctr_heap_free(((ctr_generator*)res->ptr)->data);
        ctr_heap_free(res->ptr);
        return NULL;
    case CTR_FN_OF_GENNY:
    case CTR_IFN_OF_GENNY:
    case CTR_XFN_OF_GENNY: {
        ctr_heap_free(((ctr_generator*)res->ptr)->sequence);
        ctr_argument* argm = ((ctr_generator*)res->ptr)->data;
        ctr_heap_free(argm->next);
        ctr_heap_free(argm);
        ctr_heap_free(res->ptr);
        return NULL;
    }
    }
    return res_;
}

/**
 * [Generator] foldl: [Block] accumulator: [Object]
 *
 * Folds this generator from the left (see Array::'foldl:accumulator:' for
 * details)
 */
ctr_object* ctr_generator_foldl(ctr_object* myself,
    ctr_argument* argumentList)
{
    ctr_resource* res = myself->value.rvalue;
    ctr_generator* genny = res->ptr;
    int gtype = res->type;
    ctr_object* folder = argumentList->object;
    if (!argumentList->next) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "Expected two arguments to Generator::'foldl:accumulator:'");
        return CtrStdNil;
    }
    ctr_object *result = argumentList->next->object, *old_result = CtrStdNil;
    if (!genny) {
        CtrStdFlow = ctr_build_string_from_cstring("::'next' on uninitialized generator");
        return CtrStdNil;
    }
    ctr_argument argm = { 0 }, argm2 = { 0 };
    argm.next = &argm2;

    while (1) {
        ctr_object* next = ctr_generator_internal_next(genny, gtype);
        if (genny->finished)
            break;
        if (next == generator_end_marker)
            continue;
        argm.object = result;
        argm.next->object = next;
        old_result = result;
        result = ctr_block_run(folder, &argm, folder);
        if (CtrStdFlow) {
            if (CtrStdFlow == CtrStdContinue) {
                CtrStdFlow = NULL;
                continue;
            }
            if (CtrStdFlow == CtrStdBreak) {
                CtrStdFlow = NULL;
                genny->finished = 1;
                result = old_result;
            }
            break;
        }
    }
    if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow = NULL;
    return result;
}

/**
 * [Generator] foldl: [Block]
 *
 * Folds this generator from the left (see Array::'foldl:' for details)
 */
ctr_object* ctr_generator_foldl0(ctr_object* myself,
    ctr_argument* argumentList)
{
    ctr_resource* res = myself->value.rvalue;
    ctr_generator* genny = res->ptr;
    int gtype = res->type;
    if (!argumentList || !argumentList->object) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "Expected an argument to Generator::'foldl:'");
        return CtrStdNil;
    }
    ctr_object* folder = argumentList->object;
    if (!genny) {
        CtrStdFlow = ctr_build_string_from_cstring("::'next' on uninitialized generator");
        return CtrStdNil;
    }
    ctr_object *result = NULL, *old_result = CtrStdNil;
    ctr_argument argm = { 0 }, argm2 = { 0 };
    argm.next = &argm2;

    while (1) {
        ctr_object* next = ctr_generator_internal_next(genny, gtype);
        if (genny->finished)
            break;
        if (next == generator_end_marker)
            continue;
        if (!result) {
            result = next;
            continue;
        }
        argm.object = result;
        argm.next->object = next;
        old_result = result;
        result = ctr_block_run(folder, &argm, folder);
        if (CtrStdFlow) {
            if (CtrStdFlow == CtrStdContinue) {
                CtrStdFlow = NULL;
                continue;
            }
            if (CtrStdFlow == CtrStdBreak) {
                CtrStdFlow = NULL;
                genny->finished = 1;
                result = old_result;
            }
            break;
        }
    }
    if (!result)
        CtrStdFlow = ctr_build_string_from_cstring(
            "Generator::'foldl:' called on empty generator");
    if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow = NULL;
    return result ?: CtrStdNil;
}

/**
 * [Generator] underlaying
 *
 * returns the underlaying generator of a given mapping generator
 */
ctr_object* ctr_generator_underlaying(ctr_object* myself,
    ctr_argument* argumentList)
{
    ctr_resource* res = myself->value.rvalue;
    ctr_generator* genny = res->ptr;
    if (!genny) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "::'underlaying' on uninitialized generator");
        return CtrStdNil;
    }
    int gtype = res->type;
    if (gtype != CTR_FN_OF_GENNY && gtype != CTR_IFN_OF_GENNY && gtype != CTR_XFN_OF_GENNY) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "::'underlaying' is defined only on mapping generators");
        return CtrStdNil;
    }
    ctr_mapping_generator* mgen = genny->sequence;
    ctr_generator* igen = mgen->genny;
    int igen_type = mgen->i_type;
    ctr_object* inst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_set_link_all(inst, ctr_std_generator);
    inst->release_hook = ctr_generator_free;
    inst->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
    ctr_resource* resv = inst->value.rvalue;
    resv->type = igen_type;
    resv->ptr = igen;
    return inst;
}
