/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 02:00:05 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/05 14:58:12 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static void	do_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->state_mutex);
	coder->last_compile_start = get_current_time();
	pthread_mutex_unlock(&coder->state_mutex);
	print_status(coder, COMPILE_CLR, "is compiling");
	codex_usleep(coder->env->time_to_compile, coder->env);
	pthread_mutex_lock(&coder->state_mutex);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->state_mutex);
}

static void	do_debug(t_coder *coder)
{
	print_status(coder, DEBUG_CLR, "is debugging");
	codex_usleep(coder->env->time_to_debug, coder->env);
}

static void	do_refactor(t_coder *coder)
{
	print_status(coder, REFACTOR_CLR, "is refactoring");
	codex_usleep(coder->env->time_to_refactor, coder->env);
}

static int	check_simulation_end(t_env *env)
{
	int	is_end;

	pthread_mutex_lock(&env->end_mutex);
	is_end = env->simulation_end;
	pthread_mutex_unlock(&env->end_mutex);
	return (is_end);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_env	*env;

	coder = (t_coder *)arg;
	env = coder->env;
	pthread_mutex_lock(&coder->state_mutex);
	coder->last_compile_start = env->start_time;
	pthread_mutex_unlock(&coder->state_mutex);
	while (!check_simulation_end(env))
	{
		take_dongles(coder);
		if (check_simulation_end(env))
			break ;
		do_compile(coder);
		release_dongles(coder);
		do_debug(coder);
		do_refactor(coder);
	}
	return (NULL);
}
