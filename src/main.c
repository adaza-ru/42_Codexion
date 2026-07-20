/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/28 18:26:06 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/07/20 02:22:50 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

int	main(int argc, char **argv)
{
	int	error_mark;
	t_env	env;

	error_mark = check_arg_errors(argc, argv);
	if (error_mark)
	{
		print_error(error_mark);
		return (error_mark);
	}
	error_mark = init_env(argc, argv);
	if (error_mark)
	{
		print_error(error_mark);
		return (error_mark);
	}
	error_mark = check_arg_errors(argc, argv);
	if (error_mark)
	{
		print_error(error_mark);
		return (error_mark);
	}
//	start_simulation(env)
	return (error_mark);
}
