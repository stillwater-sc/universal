#
# multi-stage build
# docker build --target builder -t stillwater/universal:builder will just build a builder container
# docker build --target release -t stillwater/universal:release will just build a release container

# BUILDER stage
FROM gcc:7 as builder
MAINTAINER Theodore Omtzigt
# create a cmake build environment
RUN apt-get update && apt-get install -y build-essential apt-utils cmake \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# make certain you have a good .dockerignore file installed so that this layer isn't ginormous
COPY . /usr/src/universal
# print contextual information of the container at this state for validation the process is solid
RUN ls -la /usr/src/universal && cmake -version 

# set up the cmake/make environment to issue the build commands
RUN mkdir build 
WORKDIR /usr/src/universal/build
RUN cmake -DBUILD_CI_CHECK=ON .. && make

# actual command 'make test' is run as part of the test pipeline

# add a command that when you run the container without a command, it produces something meaningful
CMD ["echo", "Universal Numbers Library Version 2.0.0"]


# RELEASE stage
#FROM alpine:latest as release    # hitting a segfault during startup of some playground programs
FROM debian:latest as release
MAINTAINER Theodore Omtzigt

#RUN apk add --no-cache libc6-compat libstdc++ make cmake bash gawk sed grep bc coreutils
RUN apt-get update && apt-get install -y make cmake

# after building, the test executables are organized in the build directory under root
# ctest gets its configuration for CTestTestfile.cmake files. There is one at the root of the build tree
# and one for each directory that contains test executables.
COPY --from=builder /usr/src/universal/build/tools/cmd/cmd_*             	              /usr/src/universal/build/tools/cmd/
COPY --from=builder /usr/src/universal/build/tools/cmd/*.cmake           	              /usr/src/universal/build/tools/cmd/
COPY --from=builder /usr/src/universal/build/tests/posit/posit_*         	              /usr/src/universal/build/tests/posit/
COPY --from=builder /usr/src/universal/build/tests/posit/*.cmake         	              /usr/src/universal/build/tests/posit/
COPY --from=builder /usr/src/universal/build/perf/perf_*                 	              /usr/src/universal/build/perf/
COPY --from=builder /usr/src/universal/build/perf/*.cmake                	              /usr/src/universal/build/perf/
COPY --from=builder /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_exact_test      /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_exact_test
COPY --from=builder /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_experiment      /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_experiment
COPY --from=builder /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit4          /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit4
COPY --from=builder /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit8          /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit8
COPY --from=builder /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit16         /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit16
COPY --from=builder /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit32         /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit32
COPY --from=builder /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit64         /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit64
COPY --from=builder /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit128        /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit128
COPY --from=builder /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit256        /usr/src/universal/build/c_api/shim/test/posit/c_api_shim_posit256
COPY --from=builder /usr/src/universal/build/c_api/shim/test/posit/*.cmake    	              /usr/src/universal/build/c_api/shim/test/posit/
COPY --from=builder /usr/src/universal/build/c_api/pure_c/test/posit/c_api_pure_posit8        /usr/src/universal/build/c_api/pure_c/test/posit/c_api_pure_posit8
COPY --from=builder /usr/src/universal/build/c_api/pure_c/test/posit/c_api_pure_playground    /usr/src/universal/build/c_api/pure_c/test/posit/c_api_pure_playground
COPY --from=builder /usr/src/universal/build/c_api/pure_c/test/posit/*.cmake                  /usr/src/universal/build/c_api/pure_c/test/posit/
COPY --from=builder /usr/src/universal/build/examples/blas/blas_*        	              /usr/src/universal/build/examples/blas/
COPY --from=builder /usr/src/universal/build/examples/blas/*.cmake       	              /usr/src/universal/build/examples/blas/
COPY --from=builder /usr/src/universal/build/examples/dsp/dsp_*          	              /usr/src/universal/build/examples/dsp/
COPY --from=builder /usr/src/universal/build/examples/dsp/*.cmake        	              /usr/src/universal/build/examples/dsp/
COPY --from=builder /usr/src/universal/build/education/posit/edu_*       	              /usr/src/universal/build/education/posit/
COPY --from=builder /usr/src/universal/build/education/posit/*.cmake     	              /usr/src/universal/build/education/posit/
COPY --from=builder /usr/src/universal/build/examples/playground/playgr* 	              /usr/src/universal/build/examples/playground/
COPY --from=builder /usr/src/universal/build/examples/playground/*.cmake 	              /usr/src/universal/build/examples/playground/
# the ctest configuration root and Makefile so we can execute _make test_ in the test stage of the CI/CD pipeline
COPY --from=builder /usr/src/universal/build/Makefile              		              /usr/src/universal/build/
COPY --from=builder /usr/src/universal/build/CTestTestfile.cmake   		              /usr/src/universal/build/

WORKDIR /usr/src/universal/build
# double check we have all the executables of interest
RUN find .

#CMD ["/usr/src/universal/tools/cmd/cmd_numeric_limits"]
CMD ["echo", "Universal Numbers Library Version 2.0.0"]
