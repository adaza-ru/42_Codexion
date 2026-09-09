# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/13 20:25:55 by adaza-ru          #+#    #+#              #
#    Updated: 2026/09/09 01:50:18 by adaza-ru         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        = codexion
CC          = cc
CFLAGS      = -Wall -Wextra -Werror -pthread -I include -g
RM          = rm -rf

SRCS_DIR    = src
OBJS_DIR    = obj

SRCS        = main.c\
			utils/time.c\
			utils/prints.c\
			utils/check_and_clean.c\
			init/arg_errors.c\
			init/init_env.c\
			init/start_simulation.c\
			core/watcher.c\
			core/coders.c\
			core/dongles.c\
			core/dongle_logic.c\
			heap/heap.c\
			heap/heap_utils.c\

OBJS        = $(addprefix $(OBJS_DIR)/, $(SRCS:.c=.o))
 
STATE_MAN   = .mandatory
STATE_COL   = .color
 
LOGS_DIR    = logs
LOG_ERR     = $(LOGS_DIR)/error_tests.log
LOG_MEM     = $(LOGS_DIR)/memcheck.log
LOG_HEL     = $(LOGS_DIR)/helgrind.log
LOG_TSAN    = $(LOGS_DIR)/tsan.log
LOG_ASAN    = $(LOGS_DIR)/asan.log
 
ARGS_FIFO_BURNOUT = 2 100 300 50 50 5 10 fifo
ARGS_FIFO_SUCCESS = 3 1000 50 50 50 4 20 fifo
ARGS_EDF_BURNOUT  = 2 100 300 50 50 5 10 edf
ARGS_EDF_SUCCESS  = 3 1000 50 50 50 4 20 edf
 
MEMCHECK    = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes
HELGRIND    = valgrind --tool=helgrind --history-level=full
 
.PHONY: all color clean fclean re recolor help \
		termtest logtest \
		logtest-errors logtest-memcheck logtest-helgrind logtest-tsan logtest-asan
 
all: $(STATE_MAN)
 
color: $(STATE_COL)
 
$(STATE_MAN):
	@if [ -f $(STATE_COL) ]; then \
		$(RM) $(OBJS_DIR) $(STATE_COL); \
	fi
	@$(MAKE) $(NAME)
	@touch $(STATE_MAN)
 
$(STATE_COL):
	@if [ -f $(STATE_MAN) ]; then \
		$(RM) $(OBJS_DIR) $(STATE_MAN); \
	fi
	@$(MAKE) $(NAME) CFLAGS="$(CFLAGS) -D CONFIG_COLOR"
	@touch $(STATE_COL)
 
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
 
$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
 
clean:
	$(RM) $(OBJS_DIR) $(STATE_MAN) $(STATE_COL)
 
fclean: clean
	$(RM) $(NAME)
 
re: fclean all
 
recolor: fclean color
 
help:
	@echo "\033[0;34mUsage: \"./codexion <number_of_coders> <time_to_burnout> <time_to_compile>"
	@echo "<time_to_debug> <time_to_refactor>"
	@echo "<number_of_compiles_required> <dongle_cooldown> <scheduler>\"\033[0m"
	@echo ""
	@echo "  all / color      build (plain / with colored output)"
	@echo "  termtest         quick, tool-free demo: base behaviour + a stress run"
	@echo "  logtest          full matrix (memcheck/helgrind/tsan/asan), written to logs/"
	@echo "  logtest-errors   just the argument-validation matrix, under memcheck"
	@echo "  logtest-memcheck just the memcheck matrix"
	@echo "  logtest-helgrind just the helgrind matrix"
	@echo "  logtest-tsan     just the ThreadSanitizer matrix"
	@echo "  logtest-asan     just the AddressSanitizer matrix"
 
termtest: recolor
	@echo "========================================="
	@echo "     QUICK DEMO (no tools, raw binary)    "
	@echo "========================================="
	@echo "\n[ERROR] Wrong number of arguments"
	-@./$(NAME) 4
	@echo "\n[ERROR] Invalid scheduler"
	-@./$(NAME) 4 400 200 100 100 5 50 incorrecto
	@echo "\n=== FIFO + Burnout ==="
	-@./$(NAME) $(ARGS_FIFO_BURNOUT)
	@echo "\n=== FIFO + Success ==="
	-@./$(NAME) $(ARGS_FIFO_SUCCESS)
	@echo "\n=== EDF + Burnout ==="
	-@./$(NAME) $(ARGS_EDF_BURNOUT)
	@echo "\n=== EDF + Success ==="
	-@./$(NAME) $(ARGS_EDF_SUCCESS)
	@echo "\n========================================="
	@echo "          QUICK DEMO COMPLETE             "
	@echo "========================================="
 
logtest: logtest-errors logtest-memcheck logtest-helgrind logtest-tsan logtest-asan
	@echo "\n========================================="
	@echo "         ALL LOGTEST SUITES DONE          "
	@echo "  Error tests: $(LOG_ERR)"
	@echo "  Memcheck:    $(LOG_MEM)"
	@echo "  Helgrind:    $(LOG_HEL)"
	@echo "  TSan:        $(LOG_TSAN)"
	@echo "  ASan:        $(LOG_ASAN)"
	@echo "========================================="
 
logtest-errors: re
	@mkdir -p $(LOGS_DIR)
	@echo "========================================="
	@echo "   ARGUMENT VALIDATION (memcheck)         "
	@echo "========================================="
	@echo "Logging to $(LOG_ERR)..."
	@echo "[ERROR] Wrong number of arguments" > $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) 4 >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	@echo "[ERROR] Negative argument" >> $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) 4 400 -200 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	@echo "[ERROR] Argument with letters" >> $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) 4 400 200 100 100t 5 50 fifo >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) NULL 400 200 100 100 5 50 fifo >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	@echo "[ERROR] Wrong scheduler: bad string & wrong case" >> $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) 4 400 200 100 100 5 50 incorrecto >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) 4 400 200 100 100 5 50 EDF >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	@echo "[ERROR] Overflow INT_MAX & underflow negative" >> $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) 4 400 200 100 100 5 2147483648 fifo >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) 4 400 -2147483649 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	@echo "[ERROR] Empty / blank argument" >> $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) 4 400 "" 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) 4 400 "   " 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	@echo "[ERROR] Floats" >> $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) 4 400 200,5 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) 4 400 200.5 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	@echo "[ERROR] Too many coders (>250)" >> $(LOG_ERR)
	-@$(MEMCHECK) ./$(NAME) 400 400 200 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR)
	@echo "[ERROR] Fail creating a thread (ulimit -u)" >> $(LOG_ERR)
	-@bash -c "ulimit -u 20 && $(MEMCHECK) ./$(NAME) 250 800 200 200 200 3 100 edf" >> $(LOG_ERR) 2>&1
	@if grep -qE "ERROR SUMMARY: [1-9]|definitely lost: [1-9]|indirectly lost: [1-9]" $(LOG_ERR); then \
		echo "\033[0;31m[MEMCHECK ERROR DETECTED] check $(LOG_ERR)\033[0m"; \
	else \
		echo "\033[0;32m[MEMCHECK CLEAN]\033[0m"; \
	fi
 
logtest-memcheck: re
	@mkdir -p $(LOGS_DIR)
	@echo "========================================="
	@echo "   BEHAVIOUR MATRIX (memcheck)            "
	@echo "========================================="
	@echo "=== FIFO + Burnout ===" > $(LOG_MEM)
	-@$(MEMCHECK) ./$(NAME) $(ARGS_FIFO_BURNOUT) >> $(LOG_MEM) 2>&1
	@echo "\n=== FIFO + Success ===" >> $(LOG_MEM)
	-@$(MEMCHECK) ./$(NAME) $(ARGS_FIFO_SUCCESS) >> $(LOG_MEM) 2>&1
	@echo "\n=== EDF + Burnout ===" >> $(LOG_MEM)
	-@$(MEMCHECK) ./$(NAME) $(ARGS_EDF_BURNOUT) >> $(LOG_MEM) 2>&1
	@echo "\n=== EDF + Success ===" >> $(LOG_MEM)
	-@$(MEMCHECK) ./$(NAME) $(ARGS_EDF_SUCCESS) >> $(LOG_MEM) 2>&1
	@if grep -qE "ERROR SUMMARY: [1-9]|definitely lost: [1-9]|indirectly lost: [1-9]" $(LOG_MEM); then \
		echo "\033[0;31m[MEMCHECK ERROR DETECTED] check $(LOG_MEM)\033[0m"; \
	else \
		echo "\033[0;32m[MEMCHECK CLEAN]\033[0m"; \
	fi
 
logtest-helgrind: re
	@mkdir -p $(LOGS_DIR)
	@echo "========================================="
	@echo "   BEHAVIOUR MATRIX (helgrind)            "
	@echo "========================================="
	@echo "This can take a little longer than memcheck..."
	@echo "=== FIFO + Burnout ===" > $(LOG_HEL)
	-@$(HELGRIND) ./$(NAME) $(ARGS_FIFO_BURNOUT) >> $(LOG_HEL) 2>&1
	@echo "\n=== FIFO + Success ===" >> $(LOG_HEL)
	-@$(HELGRIND) ./$(NAME) $(ARGS_FIFO_SUCCESS) >> $(LOG_HEL) 2>&1
	@echo "\n=== EDF + Burnout ===" >> $(LOG_HEL)
	-@$(HELGRIND) ./$(NAME) $(ARGS_EDF_BURNOUT) >> $(LOG_HEL) 2>&1
	@echo "\n=== EDF + Success ===" >> $(LOG_HEL)
	-@$(HELGRIND) ./$(NAME) $(ARGS_EDF_SUCCESS) >> $(LOG_HEL) 2>&1
	@if grep -qiE "ERROR SUMMARY: [1-9]|possible data race|lock order violation" $(LOG_HEL); then \
		echo "\033[0;31m[HELGRIND ERROR DETECTED] check $(LOG_HEL)\033[0m"; \
	else \
		echo "\033[0;32m[HELGRIND CLEAN]\033[0m"; \
	fi
 
logtest-tsan:
	@mkdir -p $(LOGS_DIR)
	@echo "========================================="
	@echo "   BEHAVIOUR MATRIX (ThreadSanitizer)     "
	@echo "========================================="
	@echo "Recompiling with -fsanitize=thread..."
	@$(MAKE) re CFLAGS="$(CFLAGS) -fsanitize=thread" > /dev/null
	@echo "=== FIFO + Burnout ===" > $(LOG_TSAN)
	-@./$(NAME) $(ARGS_FIFO_BURNOUT) >> $(LOG_TSAN) 2>&1
	@echo "\n=== FIFO + Success ===" >> $(LOG_TSAN)
	-@./$(NAME) $(ARGS_FIFO_SUCCESS) >> $(LOG_TSAN) 2>&1
	@echo "\n=== EDF + Burnout ===" >> $(LOG_TSAN)
	-@./$(NAME) $(ARGS_EDF_BURNOUT) >> $(LOG_TSAN) 2>&1
	@echo "\n=== EDF + Success ===" >> $(LOG_TSAN)
	-@./$(NAME) $(ARGS_EDF_SUCCESS) >> $(LOG_TSAN) 2>&1
	@if grep -q "WARNING: ThreadSanitizer" $(LOG_TSAN); then \
		echo "\033[0;31m[TSAN ERROR DETECTED] check $(LOG_TSAN)\033[0m"; \
	else \
		echo "\033[0;32m[TSAN CLEAN]\033[0m"; \
	fi
	@echo "Restoring standard build..."
	@$(MAKE) re > /dev/null
 
logtest-asan:
	@mkdir -p $(LOGS_DIR)
	@echo "========================================="
	@echo "   BEHAVIOUR MATRIX (AddressSanitizer)    "
	@echo "========================================="
	@echo "Recompiling with -fsanitize=address..."
	@$(MAKE) re CFLAGS="$(CFLAGS) -fsanitize=address" > /dev/null
	@echo "=== FIFO + Burnout ===" > $(LOG_ASAN)
	-@./$(NAME) $(ARGS_FIFO_BURNOUT) >> $(LOG_ASAN) 2>&1
	@echo "\n=== FIFO + Success ===" >> $(LOG_ASAN)
	-@./$(NAME) $(ARGS_FIFO_SUCCESS) >> $(LOG_ASAN) 2>&1
	@echo "\n=== EDF + Burnout ===" >> $(LOG_ASAN)
	-@./$(NAME) $(ARGS_EDF_BURNOUT) >> $(LOG_ASAN) 2>&1
	@echo "\n=== EDF + Success ===" >> $(LOG_ASAN)
	-@./$(NAME) $(ARGS_EDF_SUCCESS) >> $(LOG_ASAN) 2>&1
	@if grep -qE "ERROR: AddressSanitizer|ERROR: LeakSanitizer" $(LOG_ASAN); then \
		echo "\033[0;31m[ASAN ERROR DETECTED] check $(LOG_ASAN)\033[0m"; \
	else \
		echo "\033[0;32m[ASAN CLEAN]\033[0m"; \
	fi
	@echo "Restoring standard build..."
	@$(MAKE) re > /dev/null
 