/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 13:57:05 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/05 14:55:00 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static size_t	get_coder_deadline(t_env *env, int coder_id)
{
	size_t	last_start;

	pthread_mutex_lock(&env->coders[coder_id].state_mutex);
	last_start = env->coders[coder_id].last_compile_start;
	pthread_mutex_unlock(&env->coders[coder_id].state_mutex);
	return (last_start + env->time_to_burnout);
}

void	enqueue_coder(t_env *env, t_coder *coder)
{
	size_t	coder_deadline;
	int		i;
	int		j;

	i = 0;
	j = env->queue_size;
	if (env->scheduler == 0)
	{
		env->queue[env->queue_size] = coder->id;
		env->queue_size++;
		return ;
	}
	coder_deadline = coder->last_compile_start + env->time_to_burnout;
	while (i < env->queue_size
		&& get_coder_deadline(env, env->queue[i]) <= coder_deadline)
		i++;
	while (j > i)
	{
		env->queue[j] = env->queue[j - 1];
		j--;
	}
	env->queue[i] = coder->id;
	env->queue_size++;
}

void	dequeue_coder(t_env *env, t_coder *coder)
{
	int	i;

	i = 0;
	while (i < env->queue_size && env->queue[i] != coder->id)
		i++;
	if (i < env->queue_size)
	{
		while (i < env->queue_size - 1)
		{
			env->queue[i] = env->queue[i + 1];
			i++;
		}
		env->queue_size--;
	}
}
