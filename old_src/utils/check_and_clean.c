/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_and_clean.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 01:26:54 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/12 19:03:47 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

int	check_simulation_end(t_env *env)
{
	int	is_end;

	pthread_mutex_lock(&env->end_mutex);
	is_end = env->simulation_end;
	pthread_mutex_unlock(&env->end_mutex);
	return (is_end);
}

void	clean_up_everything(t_env *env)
{
	int	i;

	i = 0;
	pthread_mutex_destroy(&env->arbitrator_mutex);
	pthread_mutex_destroy(&env->write_mutex);
	pthread_mutex_destroy(&env->end_mutex);
	while (i < env->num_coders)
	{
		pthread_mutex_destroy(&env->coders[i].state_mutex);
		pthread_cond_destroy(&env->cond_coders[i]);
		i++;
	}
}
