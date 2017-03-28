CC ?= gcc
CFLAGS_common ?= -Wall -std=gnu99
CFLAGS_orig = -O0
CFLAGS_opt  = -O0 -pthread -g -pg
CFLAGS_pool = -O0 -pthread -g -pg

ifdef CHECK_LEAK
CFLAGS_common += -fsanitize=address -fno-omit-frame-pointer
endif

ifdef THREAD
CFLAGS_opt  += -D THREAD_NUM=${THREAD}
CFLAGS_pool  += -D THREAD_NUM=${THREAD}
endif

ifdef TASK_NUM
CFLAGS_pool  += -D TASK_NUM=${TASK_NUM}
endif

ifeq ($(strip $(DEBUG)),1)
CFLAGS_opt += -DDEBUG -g
CFLAGS_pool += -DDEBUG -g
endif

EXEC = phonebook_orig phonebook_opt phonebook_pool
GIT_HOOKS := .git/hooks/applied
.PHONY: all
all: $(GIT_HOOKS) $(EXEC)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

SRCS_common = main.c

tools/text_align: text_align.c tools/tool-text_align.c
	$(CC) $(CFLAGS_common) $^ -o $@

phonebook_orig: $(SRCS_common) phonebook_orig.c phonebook_orig.h
	$(CC) $(CFLAGS_common) $(CFLAGS_orig) \
		-DIMPL="\"$@.h\"" -o $@ \
		$(SRCS_common) $@.c

phonebook_opt: $(SRCS_common) phonebook_opt.c phonebook_opt.h text_align.c
	$(CC) $(CFLAGS_common) $(CFLAGS_opt) \
		-DIMPL="\"$@.h\"" -o $@ \
		$(SRCS_common) $@.c text_align.c

phonebook_pool: $(SRCS_common) phonebook_pool.c phonebook_pool.h text_align.c threadpool.c
	$(CC) $(CFLAGS_common) $(CFLAGS_pool) \
		-DIMPL="\"$@.h\"" -o $@ \
		$(SRCS_common) $@.c text_align.c threadpool.c

run: $(EXEC)
	echo 3 | sudo tee /proc/sys/vm/drop_caches
	watch -d -t "./phonebook_orig && echo 3 | sudo tee /proc/sys/vm/drop_caches"

cache-test: $(EXEC)
	perf stat --repeat 100 \
		-e cache-misses,cache-references,instructions,cycles \
		./phonebook_orig
	perf stat --repeat 100 \
		-e cache-misses,cache-references,instructions,cycles \
		./phonebook_opt
	perf stat --repeat 100 \
		-e cache-misses,cache-references,instructions,cycles \
		./phonebook_pool

output.txt: cache-test calculate
	./calculate

plot: output.txt
	gnuplot scripts/runtime.gp

calculate: calculate.c
	$(CC) $(CFLAGS_common) $^ -o $@

.PHONY: clean
clean:
	$(RM) $(EXEC) *.o perf.* \
	      	calculate orig.txt opt.txt pool.txt output.txt runtime.png align.txt
