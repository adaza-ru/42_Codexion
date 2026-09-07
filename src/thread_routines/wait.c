/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   wait.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:30:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/06 20:13:24 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static void	wait_on_signal(t_env *env, t_coder *coder, int left, int right)
{
	struct timespec	ts;
	size_t			deadline;

	deadline = env->dongle_free_at[left];
	if (env->dongle_free_at[right] > deadline)
		deadline = env->dongle_free_at[right];
	if (deadline <= get_current_time())
	{
		pthread_cond_wait(&env->cond_coders[coder->id],
			&env->arbitrator_mutex);
		return ;
	}
	ts.tv_sec = deadline / 1000;
	ts.tv_nsec = (deadline % 1000) * 1000000;
	pthread_cond_timedwait(&env->cond_coders[coder->id],
		&env->arbitrator_mutex, &ts);
}

static int	is_doomed(t_env *env, t_coder *coder, int ahead)
{

}

static int	dongles_available(t_env *env, int left, int right)
{
	size_t	now;

	if (env->dongle_taken[left] != 0 || env->dongle_taken[right] != 0)
		return (0);
	now = get_current_time();
	if (now < env->dongle_free_at[left] || now < env->dongle_free_at[right])
		return (0);
	return (1);
}

static int	can_take_dongles(t_env *env, t_coder *coder, int left, int right)
{
	int	i;
	int	other_id;
	int	ahead;

	if (!dongles_available(env, left, right))
		return(0);
	i = 0;
	ahead = 0;
	while (i < env->heap_size)
}

int	wait_for_turn(t_env *env, t_coder *coder)
{
	int				left;
	int				right;

	left = coder->id;
	right = (coder->id + 1) % env->num_coders;
	pthread_mutex_lock(&env->arbitrator_mutex);
	enqueue_coder(env, coder);
	while(!can_take_dongles(env, coder, left, right)
		&& !check_simulation_end(env))
		wait_on_signal(env, coder, left, right);
	dequeue_coder(env, coder);
	if (check_simulation_end(env))
	{
		pthread_mutex_unlock(&env->arbitrator_mutex);
		return (0);
	}
	env->dongle_taken[left] = 1;
	env->dongle_taken[right] = 1;
	pthread_mutex_unlock(&env->arbitrator_mutex);
	return(1);
}
