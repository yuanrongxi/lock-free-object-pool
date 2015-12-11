c_file		=$(wildcard *.c)
o_file		=$(patsubst %.c,%.o,$(c_file))
d_file		=$(patsubst %.c,%.d,$(c_file))

CURRENT_DIR 	:=$(shell pwd)
CC			= gcc 
CC_FLAGS	= -g -O2 -fPIC -I/usr/local/include
LD_FLAGS	= -lpthread
CC_DEPFLAGS	=-MMD -MF $(@:.o=.d) -MT $@



all: print_c print_o obj_pool

print_c:
	echo $(c_file)
print_o:
	echo $(o_file)

%.o:  %.c
	$(CC)  $(CC_FLAGS) $(CC_DEPFLAGS) -c $< -o $@

obj_pool: $(o_file)
	gcc -o obj_pool $(o_file) $(LD_FLAGS) 


.PHONY :clean

clean:
	rm -f $(o_file) $(d_file) obj_pool


-include $(wildcard $(FILE_O:.o=.d))
