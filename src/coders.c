/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 02:00:05 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/04 02:12:44 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_env	*env;

	coder = (t_coder *)arg;
	env = coder->env;

	// Inicializamos su last_compile_start al tiempo de inicio de la simulación
	pthread_mutex_lock(&coder->state_mutex);
	coder->last_compile_start = env->start_time;
	pthread_mutex_unlock(&coder->state_mutex);

	while (1)
	{
		pthread_mutex_lock(&env->end_mutex);
		if (env->simulation_end)
		{
			pthread_mutex_unlock(&env->end_mutex);
			break ;
		}
		pthread_mutex_unlock(&env->end_mutex);

		// 1. Pedir dongles al Árbitro (aquí la cola FIFO/EDF tomará el control)
		// take_dongles(coder);

		// 2. Compilar
		do_compile(coder);

		// 3. Devolver dongles al Árbitro
		// release_dongles(coder);

		// 4. Debuggear
		do_debug(coder);

		// 5. Refactorizar
		do_refactor(coder);
	}
	return (NULL);
}
