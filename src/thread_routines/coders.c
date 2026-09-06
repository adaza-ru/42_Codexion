/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 02:00:05 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/06 02:41:48 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static int	do_refactor(t_coder *coder, t_env *env)
{
	if (check_simulation_end(env))
		return (0);
	print_status(coder, REFACTOR_CLR, "is refactoring");
	codex_usleep(env->time_to_refactor, env);
	return (1);
}

static int	do_debug(t_coder *coder, t_env *env)
{
	if (check_simulation_end(env))
		return (0);
	print_status(coder, DEBUG_CLR, "is debugging");
	codex_usleep(env->time_to_debug, env);
	return (1);
}

static int	do_compile(t_coder *coder, t_env *env)
{
	if (check_simulation_end(env))
		return (0);
	pthread_mutex_lock(&coder->state_mutex);
	coder->last_compile_start = get_current_time();
	pthread_mutex_unlock(&coder->state_mutex);
	print_status(coder, COMPILE_CLR, "is compiling");
	codex_usleep(env->time_to_compile, env);
	pthread_mutex_lock(&coder->state_mutex);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->state_mutex);
	return (1);
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
		if (!take_dongles(coder))
			break ;
		if (!do_compile(coder, env))
			break ;
		release_dongles(coder);
		if (!do_debug(coder, env))
			break ;
		if (!do_refactor(coder, env))
			break ;
	}
	return (NULL);
}
