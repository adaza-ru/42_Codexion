/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:30:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/06 02:22:02 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

size_t	get_coder_deadline(t_env *env, int coder_id)
{
	size_t	last_start;

	pthread_mutex_lock(&env->coders[coder_id].state_mutex);
	last_start = env->coders[coder_id].last_compile_start;
	pthread_mutex_unlock(&env->coders[coder_id].state_mutex);
	return (last_start + env->time_to_burnout);
}

void	swap_nodes(int *a, int *b)
{
	int	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

int	has_higher_priority(t_env *env, int coder_a, int coder_b)
{
	size_t	deadline_a;
	size_t	deadline_b;

	if (env->scheduler == SCH_FIFO)
		return (env->arrival_seq[coder_a] < env->arrival_seq[coder_b]);
	deadline_a = get_coder_deadline(env, coder_a);
	deadline_b = get_coder_deadline(env, coder_b);
	if (deadline_a == deadline_b)
		return (env->arrival_seq[coder_a] < env->arrival_seq[coder_b]);
	return (deadline_a < deadline_b);
}
