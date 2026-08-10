/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 02:00:05 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/09 21:41:41 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static int	is_coder_done(t_coder *coder, t_env *env)
{
	int	done;

	if (env->num_compiles_required == -1)
		return (0);
	pthread_mutex_lock(&coder->state_mutex);
	done = (coder->compiles_done >= env->num_compiles_required);
	pthread_mutex_unlock(&coder->state_mutex);
	return (done);
}

int	check_simulation_end(t_env *env)
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
		if (is_coder_done(coder, env))
			break ;
		take_dongles(coder);
		if (check_simulation_end(env))
			break ;
		if (!do_compile(coder, env))
			break ;
		release_dongles(coder);
		if (check_simulation_end(env))
			break ;
		if (!do_debug(coder, env))
			break ;
		do_refactor(coder, env);
	}
	return (NULL);
}
