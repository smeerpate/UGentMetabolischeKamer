################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"D:/ti/ccs1010/ccs/tools/compiler/ti-cgt-c2000_20.2.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="D:/ti/Workspace/UGentMetabolischeKamer" --include_path="D:/ti/Workspace/UGentMetabolischeKamer/Common/F2837xD_headers/include" --include_path="D:/ti/Workspace/UGentMetabolischeKamer/Common/Lab_common/include" --include_path="D:/ti/ccs1010/ccs/tools/compiler/ti-cgt-c2000_20.2.1.LTS/include" --define=CPU1 -g --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"D:/ti/ccs1010/ccs/tools/compiler/ti-cgt-c2000_20.2.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="D:/ti/Workspace/UGentMetabolischeKamer" --include_path="D:/ti/Workspace/UGentMetabolischeKamer/Common/F2837xD_headers/include" --include_path="D:/ti/Workspace/UGentMetabolischeKamer/Common/Lab_common/include" --include_path="D:/ti/ccs1010/ccs/tools/compiler/ti-cgt-c2000_20.2.1.LTS/include" --define=CPU1 -g --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


