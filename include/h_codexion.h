/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   h_codexion.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 23:00:46 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/07/20 02:23:11 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef H_CODEXION_H
# define H_CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

# ifdef CONFIG_COLOR
#  define RESET          "\033[0m"
#  define TIMESTAMP_CLR  "\033[90m"
#  define ID_CLR         "\033[36m"
#  define DONGLE_CLR     "\033[33m"
#  define COMPILE_CLR    "\033[32m"
#  define DEBUG_CLR      "\033[34m"
#  define REFACTOR_CLR   "\033[35m"
#  define DEATH_CLR      "\033[31m"
# else
#  define RESET          ""
#  define TIMESTAMP_CLR  ""
#  define ID_CLR         ""
#  define DONGLE_CLR     ""
#  define COMPILE_CLR    ""
#  define DEBUG_CLR      ""
#  define REFACTOR_CLR   ""
#  define DEATH_CLR      ""
# endif

typedef struct s_coder	t_coder;

typedef struct s_env
{
	int				num_coders;
	int				time_to_burnout;
	int				time_to_compile;
	int				time_to_debug;
	int				time_to_refactor;
	int				num_compiles_required;
	int				dongle_cooldown;
	int				scheduler;
	int				heap_dongles[250];
	int				queue[250];
	int				queue_size;
	int				simulation_end;
	pthread_mutex_t	arbitrator_mutex;
	pthread_mutex_t	write_mutex;
	pthread_cond_t	cond_coders[250];
	pthread_mutex_t	end_mutex;
	t_coder			coders[250];
}	t_env;

struct s_coder
{
	int				id;
	int				compiles_done;
	long long		last_compile_start;
	pthread_mutex_t	state_mutex;
	t_env			*env;
};

int		check_arg_errors(int argc, char **argv);
void	print_error(int n);
int 	init_env(t_env *env, char **argv);

#endif