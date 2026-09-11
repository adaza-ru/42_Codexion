/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   watcher.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 23:51:55 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/11 12:14:29 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static void	burnout_stop(t_env *env, t_coder *coder)
{
	size_t	timestamp;
	int		i;

	i = 0;
	pthread_mutex_lock(&env->write_mutex);
	pthread_mutex_lock(&env->end_mutex);
	env->simulation_end = 1;
	timestamp = get_current_time() - env->start_time;
	fprintf(stdout, "%s%zu %d died of burnout%s\n",
		DEATH_CLR, timestamp, coder->id + 1, RESET);
	pthread_mutex_unlock(&env->end_mutex);
	pthread_mutex_unlock(&env->write_mutex);
	pthread_mutex_lock(&env->arbitrator_mutex);
	while (i < env->num_coders)
	{
		pthread_cond_broadcast(&env->cond_coders[i]);
		i++;
	}
	pthread_mutex_unlock(&env->arbitrator_mutex);
}

static void	simulation_achieved_stop(t_env *env)
{
	size_t	timestamp;
	int		i;

	i = 0;
	pthread_mutex_lock(&env->write_mutex);
	pthread_mutex_lock(&env->end_mutex);
	env->simulation_end = 1;
	timestamp = get_current_time() - env->start_time;
	fprintf(stdout, "%s%zu All coders completed their required compiles!%s\n",
		CLR_SUCCESS, timestamp, RESET);
	pthread_mutex_unlock(&env->end_mutex);
	pthread_mutex_unlock(&env->write_mutex);
	pthread_mutex_lock(&env->arbitrator_mutex);
	while (i < env->num_coders)
	{
		pthread_cond_broadcast(&env->cond_coders[i]);
		i++;
	}
	pthread_mutex_unlock(&env->arbitrator_mutex);
}

static int	check_coder(t_env *env, t_coder *coder, int *all_done)
{
	size_t	last_start;
	int		done;

	pthread_mutex_lock(&coder->state_mutex);
	last_start = coder->last_compile_start;
	done = coder->compiles_done;
	pthread_mutex_unlock(&coder->state_mutex);
	if ((get_current_time() + 1 - last_start) >= env->time_to_burnout)
	{
		burnout_stop(env, coder);
		return (1);
	}
	if (done < env->num_compiles_required)
		*all_done = 0;
	return (0);
}

int	check_all_coders(t_env *env)
{
	int	i;
	int	all_done;

	i = 0;
	all_done = 1;
	while (i < env->num_coders)
	{
		if (check_coder(env, &env->coders[i], &all_done))
			return (1);
		i++;
	}
	if (all_done)
	{
		simulation_achieved_stop(env);
		return (1);
	}
	return (0);
}

void	*watcher_routine(void *arg)
{
	t_env	*env;

	env = (t_env *)arg;
	while (!check_simulation_end(env))
	{
		if (check_all_coders(env))
			break ;
		codex_usleep(1, env);
	}
	return (NULL);
}
