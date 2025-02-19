# Copyright (c) 2021-2024 HPMicro
# SPDX-License-Identifier: BSD-3-Clause

sdk_compile_definitions(-D__USE_JL2101=1)

sdk_inc(.)
sdk_inc(../)
sdk_src(hpm_jl2101.c)
