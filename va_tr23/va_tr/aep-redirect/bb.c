/*
 *  This file is part of the SGX-Step enclave execution control framework.
 *
 *  Copyright (C) 2017 Jo Van Bulck <jo.vanbulck@cs.kuleuven.be>,
 *                     Raoul Strackx <raoul.strackx@cs.kuleuven.be>
 *
 *  SGX-Step is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SGX-Step is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SGX-Step. If not, see <http://www.gnu.org/licenses/>.
 */

#include <sgx_urts.h>
#include "Enclave/encl_u.h"
#include <sys/mman.h>
#include <signal.h>
#include "libsgxstep/enclave.h"
#include "libsgxstep/debug.h"
#include "libsgxstep/pt.h"

#define DBG_ENCL        1

int retval2 = 0;
int main( int argc, char **argv )
{
    

    sgx_enclave_id_t enclave_id2;
    sgx_launch_token_t token={0};
    int updated=0;
    sgx_create_enclave( "./Enclave/encl.so", /*debug=*/DBG_ENCL,&token, &updated, &enclave_id2, NULL );
    enclave_dummy_call(enclave_id2,&retval2);
    printf("r2=%d",retval2);
    getchar();
    sgx_destroy_enclave(enclave_id2);
    return 0;
}
