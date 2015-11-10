


#ifndef __INTERRUPT_CONTROLLER_H_
#define __INTERRUPT_CONTROLLER_H_

enum IRQ_NUMBERS
{
    IRQ_0 = 0, IRQ_1, IRQ_2, IRQ_3, IRQ_4, IRQ_5, IRQ_6, IRQ_7, IRQ_8, IRQ_9,
    IRQ_10, IRQ_11, IRQ_12, IRQ_13, IRQ_14, IRQ_15, IRQ_16, IRQ_17, IRQ_18, IRQ_19,
    IRQ_20, IRQ_21, IRQ_22, IRQ_23, IRQ_24, IRQ_25, IRQ_26, IRQ_27, IRQ_28, IRQ_29,
    IRQ_30, IRQ_31, IRQ_32, IRQ_33, IRQ_34, IRQ_35, IRQ_36, IRQ_37, IRQ_38, IRQ_39,
    IRQ_40, IRQ_41, IRQ_42, IRQ_43, IRQ_44, IRQ_45, IRQ_46, IRQ_47, IRQ_48, IRQ_49,
    IRQ_50, IRQ_51, IRQ_52, IRQ_53, IRQ_54, IRQ_55, IRQ_56, IRQ_57, IRQ_58, IRQ_59,
    IRQ_60, IRQ_61, IRQ_62, IRQ_63, IRQ_64, IRQ_65, IRQ_66, IRQ_67, IRQ_68, IRQ_69,
    IRQ_70, IRQ_71, IRQ_72, IRQ_73, IRQ_74, IRQ_75, IRQ_76, IRQ_77, IRQ_78, IRQ_79,
    IRQ_80, IRQ_81, IRQ_82, IRQ_83, IRQ_84, IRQ_85, IRQ_86, IRQ_87, IRQ_88, IRQ_89,
    IRQ_90, IRQ_91, IRQ_92, IRQ_93, IRQ_94, IRQ_95, IRQ_96, IRQ_97, IRQ_98, IRQ_99,

    IRQ_100, IRQ_101, IRQ_102, IRQ_103, IRQ_104, IRQ_105, IRQ_106, IRQ_107, IRQ_108, IRQ_109,
    IRQ_110, IRQ_111, IRQ_112, IRQ_113, IRQ_114, IRQ_115, IRQ_116, IRQ_117, IRQ_118, IRQ_119,
    IRQ_120, IRQ_121, IRQ_122, IRQ_123, IRQ_124, IRQ_125, IRQ_126, IRQ_127, IRQ_128, IRQ_129,
    IRQ_130, IRQ_131, IRQ_132, IRQ_133, IRQ_134, IRQ_135, IRQ_136, IRQ_137, IRQ_138, IRQ_139,
    IRQ_140, IRQ_141, IRQ_142, IRQ_143, IRQ_144, IRQ_145, IRQ_146, IRQ_147, IRQ_148, IRQ_149,
    IRQ_150, IRQ_151, IRQ_152, IRQ_153, IRQ_154, IRQ_155, IRQ_156, IRQ_157, IRQ_158, IRQ_159,
    IRQ_160, IRQ_161, IRQ_162, IRQ_163, IRQ_164, IRQ_165, IRQ_166, IRQ_167, IRQ_168, IRQ_169,
    IRQ_170, IRQ_171, IRQ_172, IRQ_173, IRQ_174, IRQ_175, IRQ_176, IRQ_177, IRQ_178, IRQ_179,
    IRQ_180, IRQ_181, IRQ_182, IRQ_183, IRQ_184, IRQ_185, IRQ_186, IRQ_187, IRQ_188, IRQ_189,
    IRQ_190, IRQ_191, IRQ_192, IRQ_193, IRQ_194, IRQ_195, IRQ_196, IRQ_197, IRQ_198, IRQ_199,

    IRQ_200, IRQ_201, IRQ_202, IRQ_203, IRQ_204, IRQ_205, IRQ_206, IRQ_207, IRQ_208, IRQ_209,
    IRQ_210, IRQ_211, IRQ_212, IRQ_213, IRQ_214, IRQ_215, IRQ_216, IRQ_217, IRQ_218, IRQ_219,
    IRQ_220, IRQ_221, IRQ_222, IRQ_223, IRQ_224, IRQ_225, IRQ_226, IRQ_227, IRQ_228, IRQ_229,
    IRQ_230, IRQ_231, IRQ_232, IRQ_233, IRQ_234, IRQ_235, IRQ_236, IRQ_237, IRQ_238, IRQ_239,
    IRQ_240, IRQ_241, IRQ_242, IRQ_243, IRQ_244, IRQ_245, IRQ_246, IRQ_247, IRQ_248, IRQ_249,
    IRQ_250, IRQ_251, IRQ_252, IRQ_253, IRQ_254, IRQ_255,
    MAX_NUM_IRQS = 256
};

typedef struct interrupt_controller_t interrupt_controller_t;

interrupt_controller_t* make_interrupt_controller(uint32_t ivt_start_address);
void destroy_interrupt_controller(interrupt_controller_t* ic);

void request_interrupt(interrupt_controller_t* ic, uint8_t irq_number);

bool interrupt_requested(interrupt_controller_t* ic);
uint8_t get_interrupt_source(interrupt_controller_t* ic);
uint32_t get_interrupt_vector_table_starting_address(interrupt_controller_t* ic);

#endif
