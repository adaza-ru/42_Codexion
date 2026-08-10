# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/13 20:25:55 by adaza-ru          #+#    #+#              #
#    Updated: 2026/08/10 01:43:41 by adaza-ru         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        = codexion
CC          = cc
CFLAGS      = -Wall -Wextra -Werror -I include -g
RM          = rm -rf

SRCS_DIR    = src
OBJS_DIR    = obj

SRCS        = main.c\
			utils/time.c\
			utils/prints.c\
			utils/clean_up.c\
			init/arg_errors.c\
			init/init_env.c\
			init/start_simulation.c\
			core/watcher.c\
			core/coders.c\
			core/coder_actions.c\
			core/dongles.c\
			utils/queue.c

OBJS        = $(addprefix $(OBJS_DIR)/, $(SRCS:.c=.o))

STATE_MAN   = .mandatory
STATE_COL   = .color

LOG_ERR     = logs/error_tests.log
LOG_HEL     = logs/helgrind.log
LOG_TSAN    = logs/tsan.log
LOG_MEM 	= logs/memcheck.log

.PHONY: all color clean fclean re recolor termtest logtest help

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
	@echo "<time_to_compile> <time_to_debug> <time_to_refactor>"
	@echo "<number_of_compiles_required> <dongle_cooldown> <scheduler>\"\n\033[0m"

termtest: $(NAME)
	@$(MAKE) recolor CFLAGS="$(CFLAGS)" > /dev/null
	@echo "Cleaning previous logs..."
	@$(RM) logs
	
	@echo "========================================="
	@echo "      1/4. RUNNING PARSER/INIT TESTS     "
	@echo "========================================="
	
	@echo "\n[ERROR] Wrong number of arguments"
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4
	
	@echo "\n[ERROR] Negative arguments"
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 -200 100 100 5 50 edf
	
	@echo "\n[ERROR] Argument with letters"
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 200 100 100t 5 50 fifo
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) NULL 400 200 100 100 5 50 fifo

	@echo "\n[ERROR] Wrong scheduler: Incorrect string & Uppercase"
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 200 100 100 5 50 incorrecto
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 200 100 100 5 50 EDF
	
	@echo "\n[ERROR] Overflow INT_MAX & Underflow INT_MIN"
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 200 100 100 5 2147483648 fifo
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 -2147483649 100 100 5 50 edf

	@echo "\n[ERROR] Empty argument"
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 "" 100 100 5 50 edf
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 "   " 100 100 5 50 edf

	@echo "\n[ERROR] Floats"
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 200,5 100 100 5 50 edf
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 200.5 100 100 5 50 edf

	@echo "\n[ERROR] Fail creating a thread"
	-@bash -c "ulimit -u 20 && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 250 800 200 200 200 3 100 edf"


	@echo "\n\n========================================="
	@echo "      2/4. RUNNING MEMCHECK TESTS        "
	@echo "========================================="
	@echo "\n=== [TEST 1] FIFO + Burnout ==="
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 5 600 200 200 200 10 50 fifo
	@echo "\n=== [TEST 2] FIFO + Success ==="
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 1000 200 200 200 5 50 fifo
	@echo "\n=== [TEST 3] EDF + Burnout ==="
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 5 600 200 200 200 10 50 edf
	@echo "\n=== [TEST 4] EDF + Success ==="
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 1000 200 200 200 5 50 edf



	@echo "\n\n========================================="
	@echo "      3/4. RUNNING HELGRIND TESTS        "
	@echo "========================================="
	@echo "Executing 4 matrix tests with Helgrind (may take a few seconds)..."
	@echo "\n=== [TEST 1] FIFO + Burnout ==="
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 5 600 200 200 200 10 50 fifo
	@echo "\n=== [TEST 2] FIFO + Success ==="
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 4 1000 200 200 200 5 50 fifo
	@echo "\n=== [TEST 3] EDF + Burnout ==="
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 5 600 200 200 200 10 50 edf
	@echo "\n=== [TEST 4] EDF + Success ==="
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 4 1000 200 200 200 5 50 edf
	@echo ""

	@echo "\n\n========================================="
	@echo "   4/4. RUNNING THREADSANITIZER TESTS    "
	@echo "========================================="
	@echo "Recompiling binary with ThreadSanitizer flags..."
	@$(MAKE) recolor CFLAGS="$(CFLAGS) -fsanitize=thread" > /dev/null
	@echo "\n=== [TEST 1] FIFO + Burnout ==="
	-@./$(NAME) 5 600 200 200 200 10 50 fifo
	@echo "\n=== [TEST 2] FIFO + Success ==="
	-@./$(NAME) 4 1000 200 200 200 5 50 fifo
	@echo "\n=== [TEST 3] EDF + Burnout ==="
	-@./$(NAME) 5 600 200 200 200 10 50 edf
	@echo "\n=== [TEST 4] EDF + Success ==="
	-@./$(NAME) 4 1000 200 200 200 5 50 edf
	@echo "\nRestoring binary to standard compilation state..."
	@$(MAKE) recolor > /dev/null
	
	@echo "\n========================================"
	@echo "        ALL TESTS COMPLETED              "
	@echo "========================================="



logtest: $(NAME)
	@$(MAKE) re CFLAGS="$(CFLAGS)" > /dev/null
	@echo "Cleaning previous logs..."
	@$(RM) logs
	@mkdir logs
	
	@echo "========================================="
	@echo "      1/4. RUNNING PARSER/INIT TESTS     "
	@echo "========================================="
	@echo "Writing error cases output to '$(LOG_ERR)'..."
	
	@echo "\n[ERROR] Wrong number of arguments" > $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1

	@echo "\n[ERROR] Negative arguments" >> $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 -200 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1

	@echo "\n[ERROR] Argument with letters" >> $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 200 100 100t 5 50 fifo >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) NULL 400 200 100 100 5 50 fifo >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1

	@echo "\n[ERROR] Wrong scheduler: Incorrect string & Uppercase" >> $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 200 100 100 5 50 incorrecto >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 200 100 100 5 50 EDF >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1
	
	@echo "\n[ERROR] Overflow INT_MAX & Underflow INT_MIN" >> $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 200 100 100 5 2147483648 fifo >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 -2147483649 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1

	@echo "\n[ERROR] Empty argument" >> $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 "" 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 "   " 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1

	@echo "\n[ERROR] Floats" >> $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 200,5 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 400 200.5 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1

	@echo "\n[ERROR] Too many programmers" >> $(LOG_ERR) 2>&1
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 400 400 200 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	@echo "" >> $(LOG_ERR) 2>&1

	@if grep -E "ERROR SUMMARY: [1-9]|definitely lost:|definitely lost: [1-9]" $(LOG_ERR) > /dev/null 2>&1; then \
		echo "\033[0;31m[MEMCHECK ERROR DETECTED] Check $(LOG_ERR)\033[0m"; \
	else \
		echo "\033[0;32m[MEMCHECK CLEAN]\033[0m"; \
	fi


	@echo "\n\n========================================="
	@echo "      2/4. RUNNING MEMCHECK TESTS        "
	@echo "========================================="
	@echo "Executing 4 matrix tests with Memcheck, logging to '$(LOG_MEM)'..."
	@echo "\n=== [TEST 1] FIFO + Burnout ===" > $(LOG_MEM)
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 5 600 200 200 200 10 50 fifo >> $(LOG_MEM) 2>&1
	@echo "\n=== [TEST 2] FIFO + Success ===" >> $(LOG_MEM)
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 1000 200 200 200 5 50 fifo >> $(LOG_MEM) 2>&1
	@echo "\n=== [TEST 3] EDF + Burnout ===" >> $(LOG_MEM)
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 5 600 200 200 200 10 50 edf >> $(LOG_MEM) 2>&1
	@echo "\n=== [TEST 4] EDF + Success ===" >> $(LOG_MEM)
	-@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 4 1000 200 200 200 5 50 edf >> $(LOG_MEM) 2>&1
	@if grep -E "ERROR SUMMARY: [1-9]|definitely lost:|definitely lost: [1-9]" $(LOG_MEM) > /dev/null 2>&1; then \
		echo "\033[0;31m[MEMCHECK ERROR DETECTED] Check $(LOG_MEM)\033[0m"; \
	else \
		echo "\033[0;32m[MEMCHECK CLEAN]\033[0m"; \
	fi


	@echo "\n\n========================================="
	@echo "      3/4. RUNNING HELGRIND TESTS        "
	@echo "========================================="
	@echo "Executing 4 matrix tests with Helgrind (may take a few seconds)..."
	@echo "Logging to '$(LOG_HEL)'..."
	@echo "\n=== [TEST 1] FIFO + Burnout ===" > $(LOG_HEL)
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 5 600 200 200 200 10 50 fifo >> $(LOG_HEL) 2>&1
	@echo "\n=== [TEST 2] FIFO + Success ===" >> $(LOG_HEL)
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 4 1000 200 200 200 5 50 fifo >> $(LOG_HEL) 2>&1
	@echo "\n=== [TEST 3] EDF + Burnout ===" >> $(LOG_HEL)
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 5 600 200 200 200 10 50 edf >> $(LOG_HEL) 2>&1
	@echo "\n=== [TEST 4] EDF + Success ===" >> $(LOG_HEL)
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 4 1000 200 200 200 5 50 edf >> $(LOG_HEL) 2>&1
	@if grep -E "ERROR SUMMARY: [1-9]|possible data race|lock order violation" $(LOG_HEL) > /dev/null 2>&1; then \
		echo "\033[0;31m[HELGRIND ERROR DETECTED] Check $(LOG_HEL)\033[0m"; \
	else \
		echo "\033[0;32m[HELGRIND CLEAN]\033[0m"; \
	fi
	@echo ""

	@echo "\n\n========================================="
	@echo "   4/4. RUNNING THREADSANITIZER TESTS    "
	@echo "========================================="
	@echo "Recompiling binary with ThreadSanitizer flags..."
	@$(MAKE) re CFLAGS="$(CFLAGS) -fsanitize=thread" > /dev/null
	@echo "Executing 4 matrix tests with TSan, logging to '$(LOG_TSAN)'..."
	@echo "\n=== [TEST 1] FIFO + Burnout ===" > $(LOG_TSAN)
	-@./$(NAME) 5 600 200 200 200 10 50 fifo >> $(LOG_TSAN) 2>&1
	@echo "\n=== [TEST 2] FIFO + Success ===" >> $(LOG_TSAN)
	-@./$(NAME) 4 1000 200 200 200 5 50 fifo >> $(LOG_TSAN) 2>&1
	@echo "\n=== [TEST 3] EDF + Burnout ===" >> $(LOG_TSAN)
	-@./$(NAME) 5 600 200 200 200 10 50 edf >> $(LOG_TSAN) 2>&1
	@echo "\n=== [TEST 4] EDF + Success ===" >> $(LOG_TSAN)
	-@./$(NAME) 4 1000 200 200 200 5 50 edf >> $(LOG_TSAN) 2>&1
	@if grep -E "WARNING: ThreadSanitizer|data race|deadlock" $(LOG_TSAN) > /dev/null 2>&1; then \
		echo "\033[0;31m[TSAN ERROR DETECTED] Check $(LOG_TSAN)\033[0m"; \
	else \
		echo "\033[0;32m[TSAN CLEAN]\033[0m"; \
	fi
	@echo "\nRestoring binary to standard compilation state..."
	@$(MAKE) recolor > /dev/null
	
	@echo "\n========================================="
	@echo "        ALL TESTS COMPLETED              "
	@echo " Logs saved in:"
	@echo "   - Error Tests:    $(LOG_ERR)"
	@echo "   - Memcheck Tests: $(LOG_MEM)"
	@echo "   - Helgrind:       $(LOG_HEL)"
	@echo "   - TSan:           $(LOG_TSAN)"
	@echo "========================================="