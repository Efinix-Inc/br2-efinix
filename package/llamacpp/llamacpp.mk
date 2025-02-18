################################################################################
#
# llamacpp
#
################################################################################

LLAMACPP_VERSION = b4735
LLAMACPP_DESCRIPTION = LLM inference in C/C++
LLAMACPP_SITE = $(call github,ggml-org,llama.cpp,$(LLAMACPP_VERSION))
LLAMACPP_LICENSE = MIT
LLAMACPP_LICENSE_FILES = LICENSE

LLAMACPP_MODEL = smollm2-360m-instruct-q8_0.gguf
LLAMACPP_EXTRA_DOWNLOADS = https://huggingface.co/mitulagr2/gguf-Q8_0-SmolLM2-360M-Instruct/blob/main/$(LLAMACPP_MODEL)

LLAMACPP_CONF_OPTS = \
        -DAPPLE=NO \
        -DGGML_ACCELERATE=OFF \
        -DGGML_CPU_AARCH64=OFF \
        -DGGML_RVV=OFF \
        -DGGML_BUILD_EXAMPLES=OFF \
        -DGGML_BUILD_TESTS=OFF \
        -DGGML_OPENCL_USE_ADRENO_KERNELS=OFF \

ifeq ($(BR2_GCC_ENABLE_OPENMP),y)
LLAMACPP_CONF_OPTS += -DGGML_OPENMP=ON
endif

define LLAMACPP_EXTRACT_MODEL
        mkdir -p $(TARGET_DIR)/usr/share/llamacpp/model/guff && \
        $(INSTALL) -D -m 0644 $(LLAMACPP_DL_DIR)/$(LLAMACPP_MODEL) $(TARGET_DIR)/usr/share/llamacpp/model/guff/$(LLAMACPP_MODEL)
endef

LLAMACPP_POST_EXTRACT_HOOKS += LLAMACPP_EXTRACT_MODEL

$(eval $(cmake-package))
