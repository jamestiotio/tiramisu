#include <isl/set.h>
#include <isl/union_map.h>
#include <isl/union_set.h>
#include <isl/ast_build.h>
#include <isl/schedule.h>
#include <isl/schedule_node.h>

#include <coli/debug.h>
#include <coli/core.h>

#include <string.h>
#include <Halide.h>

using namespace coli;

int main(int argc, char **argv)
{
    // Set default coli options.
    global::set_default_coli_options();

    // Declare a function.
    // A function in coli is the equivalent of a function in C.
    // It can have input and output buffers.  The buffers are declared later.
    function function0("function0");

    // Create a buffer buf0.  This buffer is supposed to be allocated outside
    // the function "function0" and passed to it as an argument.
    // Buffers of type a_output or type::argument::input
    // should be allocated outside the function and passed as arguments to the
    // function.  Buffers of type type::argument::temporary are
    // allocated automatically by coli within the function and thus should
    // not be passed as arguments to the function.
    buffer buf0("buf0", 2, {coli::expr(10),coli::expr(10)}, p_uint8, NULL, a_output, &function0);

    // Declare the invariants of the function.  An invariant can be a symbolic
    // constant or a variable that does not change during the execution of the
    // function.
    invariant N("N", expr((int32_t) 10), &function0);

    // Declare a expressions that will be associated with the
    // computations.
    expr e3 = expr((uint8_t) 3) + expr((uint8_t) 4);
    // Declare a computation within function0.
    // To declare a computation, you need to provide:
    // (1) an ISL set representing the iteration space of the computation,
    // coli uses the ISL syntax for sets and maps.  This syntax is described
    // in http://barvinok.gforge.inria.fr/barvinok.pdf (Sections
    // 1.2.1 for sets and iteration domains, and 1.2.2 for maps and access
    // relations),
    // (2) a coli expression that represents the computation,
    // (3) the function in which the computation will be declared.
    computation S0("[N]->{S0[i,j]: 0<=i<N and 0<=j<N}", &e3, true, p_uint8, &function0);

    // Map the computations to a buffer (i.e. where each computation
    // should be stored in the buffer).
    // This mapping will be updated automaticall when the schedule
    // is applied.  To disable automatic data mapping updates use
    // global::set_auto_data_mapping(false).
    S0.set_access("{S0[i,j]->buf0[i,j]}");

    // Dump the iteration domain (input) for the function.
    function0.dump_iteration_domain();

    // Set the schedule of each computation.
    // The identity schedule means that the program order is not modified
    // (i.e. no optimization is applied).
    S0.tile(0,1,2,2);
    S0.tag_parallel_dimension(0);

    // Dump the schedule.
    function0.dump_schedule();

    // Add buf0 as an argument to the function.
    function0.set_arguments({&buf0});

    // Generate the time-processor domain of the computation
    // and dump it on stdout.
    // This is purely for debugging, the tim-processor representation
    // is not used later on.
    function0.gen_time_processor_domain();
    function0.dump_time_processor_domain();

    // Generate an AST (abstract Syntax Tree)
    function0.gen_isl_ast();

    // Generate Halide statement for the function.
    function0.gen_halide_stmt();

    // If you want to get the generated halide statements, call
    // fct.get_halide_stmts().

    // Dump the Halide stmt generated by gen_halide_stmt()
    // for the function.
    function0.dump_halide_stmt();

    // Generate an object file from the function.
    function0.gen_halide_obj("build/generated_fct_tutorial_01.o");

    return 0;
}

/**
 * Assumptions
 * - Note that the name used during the construction of a coli object and the
 *   identifier of that object are identical (for example buf0, "buf0").
 *   This is not required but highly recommended as it simplifies reading
 *   coli code and prevents errors.
 * -
 *
 * Current limitations:
 *  - Note that the type of the invariant N is "int32_t".  This is important
      because this invariant is used later as a loop bound and the
      type of the bound and the iterator should be the same for correct code
      generation.  This implies that the invariant should be of type "int32_t".

 */
