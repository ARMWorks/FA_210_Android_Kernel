# Samsung Platform Support
snd-soc-s5pv2xx-objs := dma.o
snd-soc-s5pc1xx-i2s-objs := s5pc1xx-i2s.o
snd-soc-s5p-idma-objs := ss3c-dma-wrapper.o 3c-idma.o s5p-i2s_sec.o

obj-$(CONFIG_SND_S5PV2XX_SOC) += snd-soc-s5pv2xx.o
obj-$(CONFIG_SND_S5PC1XX_I2S) += snd-soc-s5pc1xx-i2s.o
obj-$(CONFIG_S5P_INTERNAL_DMA) += snd-soc-s5p-idma.o

# S5PV2XX Machine Support
snd-soc-mini210-wm8960-objs := mini210_wm8960.o

obj-$(CONFIG_SND_S5PV2XX_SOC_WM8960) += snd-soc-mini210-wm8960.o

