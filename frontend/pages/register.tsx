import React, { useState } from 'react';
import { useRouter } from 'next/router';
import Head from 'next/head';
import Link from 'next/link';
import FormInput from '../components/FormInput';
import { RegisterFormData } from '../types/auth';

const Register: React.FC = () => {
  const router = useRouter();
  const [formData, setFormData] = useState<RegisterFormData>({
    username: '',
    password: '',
    confirmPassword: '',
  });
  const [error, setError] = useState<string>('');

  const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const { name, value } = e.target;
    setFormData(prev => ({
      ...prev,
      [name]: value,
    }));
  };

  const validateForm = (): boolean => {
    if (formData.password !== formData.confirmPassword) {
      setError('两次输入的密码不一致');
      return false;
    }
    if (formData.password.length < 6) {
      setError('密码长度至少为6位');
      return false;
    }
    return true;
  };

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setError('');

    if (!validateForm()) {
      return;
    }

    try {
      const response = await fetch('http://localhost:9000/api/register', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          username: formData.username,
          password: formData.password,
        }),
      });

      const data = await response.json();

      if (data.success) {
        // 注册成功后跳转到登录页面
        router.push('/login');
      } else {
        setError(data.message || '注册失败');
      }
    } catch (err) {
      setError('服务器错误，请稍后重试');
    }
  };

  return (
    <div className="min-h-screen flex items-center justify-center bg-gray-50 py-12 px-4 sm:px-6 lg:px-8">
      <Head>
        <title>注册 - Web Server</title>
      </Head>

      <div className="max-w-md w-full space-y-8">
        <div>
          <h2 className="mt-6 text-center text-3xl font-extrabold text-gray-900">
            创建新账号
          </h2>
        </div>
        <form className="mt-8 space-y-6" onSubmit={handleSubmit}>
          <div className="rounded-md shadow-sm -space-y-px">
            <FormInput
              label="用户名"
              type="text"
              name="username"
              value={formData.username}
              onChange={handleChange}
              required
              error={error}
            />
            <FormInput
              label="密码"
              type="password"
              name="password"
              value={formData.password}
              onChange={handleChange}
              required
            />
            <FormInput
              label="确认密码"
              type="password"
              name="confirmPassword"
              value={formData.confirmPassword}
              onChange={handleChange}
              required
            />
          </div>

          <div>
            <button
              type="submit"
              className="group relative w-full flex justify-center py-2 px-4 border border-transparent text-sm font-medium rounded-md text-white bg-blue-600 hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500"
            >
              注册
            </button>
          </div>

          <div className="text-sm text-center">
            <Link href="/login">
              <a className="font-medium text-blue-600 hover:text-blue-500">
                已有账号？立即登录
              </a>
            </Link>
          </div>
        </form>
      </div>
    </div>
  );
};

export default Register; 