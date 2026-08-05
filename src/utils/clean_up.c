/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   clean_up.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 01:26:54 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/05 00:04:58 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

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
